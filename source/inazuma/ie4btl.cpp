#include <stdint.h>
#include "../inazuma/ie4btl.h"

static_assert(sizeof(BtlScriptArg) == 8, "BtlScriptArg must be 8 bytes");
static_assert(sizeof(BtlTeamSetup) == 80, "BtlTeamSetup must be 80 bytes");
static_assert(__builtin_offsetof(BtlTeamSetup, CoachHash) == 16, "BtlTeamSetup layout mismatch");
static_assert(__builtin_offsetof(BtlTeamSetup, EmblemId) == 20, "BtlTeamSetup layout mismatch");
static_assert(__builtin_offsetof(BtlTeamSetup, TeamName) == 40, "BtlTeamSetup layout mismatch");
static_assert(sizeof(BtlEventContext) == 20, "BtlEventContext must be 20 bytes");
static_assert(sizeof(SoccerInfo) == 20, "SoccerInfo must be 20 bytes");
static_assert(sizeof(BtlForceEntry) == 8, "BtlForceEntry must be 8 bytes");
static_assert(__builtin_offsetof(SoccerInfo, Mode) == 16, "SoccerInfo layout mismatch");

/** @brief Script function called when the ball carrier enters the penalty area of the other team */
#define ENTER_PENALTY_AREA_CALLBACK "CallMatchEnterPenaltyArea"

/**
 * @brief Per team, the bit of the player slot that stood in the other team's penalty area with the ball last frame
 * @details The linker script folds .bss into .text, so this starts out zeroed.
 */
static uint32_t sInPenaltyArea[2];

/** @brief Live state buffer of a match */
static int liveState(int match) {
    return match + BTL_MATCH_STATE_BASE
        + BTL_MATCH_STATE_SIZE * *reinterpret_cast<int32_t *>(match + BTL_MATCH_STATE_INDEX);
}

void ie4BtlRefreshPlayerParams(void) {
    int match = *reinterpret_cast<int *>(BTL_MATCH_OFFSET);

    if (match == 0) {
        return;
    }

    btlRecalcPlayerParams(match, liveState(match), 0);
}

/**
 * @brief Tells whether a challenge with this victory condition can go on after a goal
 */
static bool challengeGoesOnAfterGoal(uint8_t victory) {
    return victory == BTL_VICTORY_LEAD_2 || victory == BTL_VICTORY_LEAD_3 || victory == BTL_VICTORY_CLEAN_SHEET;
}

/**
 * @brief Runs at the start of every match, before InitBtl and BtlSetting
 * @details Called by ie4BtlOnMatchSetupAsm, drops everything left over from the
 * previous match.
 */
extern "C" void ie4BtlOnMatchSetup(void) {
    sInPenaltyArea[0] = 0;
    sInPenaltyArea[1] = 0;
}

/**
 * @brief Fires ENTER_PENALTY_AREA_CALLBACK when the ball carrier stands in the
 * penalty area of the other team and did not last frame
 * @details Covers dribbling into the area as well as getting the ball inside
 * it; keeping the ball in the area fires nothing more.
 */
static void checkPenaltyAreas(int match) {
    int *script = *reinterpret_cast<int **>(match + BTL_MATCH_SCRIPT);

    // Leave everything as is while the VM sits in another callback, the entries
    // still standing get reported once it is free
    if (script == 0 || *script == 0 || *reinterpret_cast<uint8_t *>(reinterpret_cast<int>(script) + 4)) {
        return;
    }

    int state = liveState(match);
    const float *field = reinterpret_cast<const float *>(match + BTL_MATCH_FIELD);
    uint32_t holder = *reinterpret_cast<uint8_t *>(state + BTL_STATE_BALL_HOLDER);

    for (uint32_t team = 0; team < 2; team++) {
        uint8_t *teamData = reinterpret_cast<uint8_t *>(state + BTL_STATE_TEAMS + BTL_STATE_TEAM_SIZE * team);
        int8_t direction = static_cast<int8_t>(teamData[BTL_TEAM_DIRECTION]);
        uint32_t inside = 0;

        // Direction is 0 before the teams are placed, and the area test would
        // then pick the middle of the field
        if (direction != 0) {
            float side = -static_cast<float>(direction);

            for (uint32_t i = 0; i < teamData[BTL_TEAM_FIELD_COUNT]; i++) {
                uint32_t slot = static_cast<uint8_t>(teamData[BTL_TEAM_FIRST_SLOT] + i);

                // Only the ball carrier counts, and nobody while the ball is loose
                if (slot >= BTL_STATE_PLAYER_COUNT || slot != holder) {
                    continue;
                }

                int player = state + BTL_STATE_PLAYERS + BTL_STATE_PLAYER_SIZE * slot;

                if (btlIsInPenaltyArea(field, reinterpret_cast<const void *>(player + BTL_PLAYER_POSITION), side, 0.0f)) {
                    inside |= 1u << slot;
                }
            }
        }

        uint32_t entered = inside & ~sInPenaltyArea[team];
        sInPenaltyArea[team] = inside;

        for (uint32_t slot = 0; entered != 0; slot++, entered >>= 1) {
            if ((entered & 1) == 0) {
                continue;
            }

            int player = state + BTL_STATE_PLAYERS + BTL_STATE_PLAYER_SIZE * slot;
            BtlScriptArg args[2];

            args[0].Type = BTL_SCRIPT_ARG_INT;
            args[0].Value = team;
            args[1].Type = BTL_SCRIPT_ARG_INT;
            args[1].Value = *reinterpret_cast<uint32_t *>(player + BTL_PLAYER_PARAM_ID);

            callBtlScriptFunc(script, ENTER_PENALTY_AREA_CALLBACK, args, 2);
        }
    }
}

/**
 * @brief Victory condition of a match, BTL_VICTORY_NONE when it is not a challenge
 * @param field Field geometry of the match
 */
static uint8_t challengeVictory(const uint8_t *field) {
    if (*reinterpret_cast<const uint16_t *>(field + BTL_FIELD_KIND) != BTL_FIELD_KIND_CHALLENGE) {
        return BTL_VICTORY_NONE;
    }

    return field[BTL_FIELD_VICTORY];
}

/** @brief Empties a force list */
__attribute__((optimize("no-tree-loop-distribute-patterns")))
static void clearForceList(BtlForceEntry *list) {
    uint32_t i;

    for (i = 0; i < BTL_FORCE_LIST_SIZE; i++) {
        list[i].Id = 0;
        list[i].Team = -1;
        list[i].Unk6 = 0;
    }
}

/**
 * @brief Turns the goalkeeper of a team into a defender who leaves his goal
 * @param team Team entry, see BTL_STATE_TEAMS
 * @details He gets the role of a defender, the team is left without a
 * goalkeeper, and his home position in every formation becomes the middle of
 * those of his team mates. Doing it again changes nothing, the team has no
 * goalkeeper any more.
 */
__attribute__((optimize("no-tree-loop-distribute-patterns")))
static void releaseKeeper(uint8_t *team) {
    uint32_t keeper = team[BTL_TEAM_KEEPER];
    uint32_t count = team[BTL_TEAM_FIELD_COUNT];
    uint32_t formations = team[BTL_TEAM_FORMATION_COUNT];

    if (keeper >= count || count < 2) {
        return;
    }

    uint16_t *role = reinterpret_cast<uint16_t *>(team + BTL_TEAM_ROLES + BTL_TEAM_ROLE_SIZE * keeper);

    role[0] = BTL_ROLE_DEFENDER;
    role[1] = BTL_ROLE_FLAG_DEFENDER;
    team[BTL_TEAM_KEEPER] = 0xFF;

    if (formations > BTL_TEAM_FORMATION_MAX) {
        formations = BTL_TEAM_FORMATION_MAX;
    }

    for (uint32_t i = 0; i < formations; i++) {
        float *position = reinterpret_cast<float *>(team + BTL_TEAM_FORMATIONS + BTL_TEAM_FORMATION_SIZE * i);
        float x = 0.0f;
        float z = 0.0f;

        for (uint32_t player = 0; player < count; player++) {
            if (player != keeper) {
                x += position[2 * player];
                z += position[2 * player + 1];
            }
        }

        position[2 * keeper] = x / static_cast<float>(count - 1);
        position[2 * keeper + 1] = z / static_cast<float>(count - 1);
    }
}

/**
 * @brief Opens and closes the goals the way the conditions of this module need
 * @details The challenge script puts the goalkeeper of team 0 in the force win
 * list, which closes his goal with a barrier and makes him stop every shot, so
 * nobody can score against team 0. A lead has to be possible both ways, so both
 * force lists are emptied every frame. A clean sheet leaves team 1 without a
 * goal: ie4BtlChallengeWalls walls its goal line in and team 0 cannot shoot,
 * so its goalkeeper joins the others.
 */
static void applyChallengeGoals(int match) {
    uint8_t victory = challengeVictory(reinterpret_cast<const uint8_t *>(match + BTL_MATCH_FIELD));

    if (!challengeGoesOnAfterGoal(victory)) {
        return;
    }

    clearForceList(reinterpret_cast<BtlForceEntry *>(match + BTL_MATCH_FORCE_WIN));
    clearForceList(reinterpret_cast<BtlForceEntry *>(match + BTL_MATCH_FORCE_LOSE));

    if (victory == BTL_VICTORY_CLEAN_SHEET) {
        releaseKeeper(reinterpret_cast<uint8_t *>(liveState(match) + BTL_STATE_TEAMS + BTL_STATE_TEAM_SIZE));
    }
}

/**
 * @brief Stands in for the btlMatchStepUnk call of the match update
 * @details Hooked in place of that call, so it runs once per frame of live play.
 */
extern "C" uint32_t ie4BtlMatchStep(int match) {
    uint32_t result = btlMatchStepUnk(match);

    checkPenaltyAreas(match);
    applyChallengeGoals(match);

    return result;
}

/**
 * @brief Runs every frame right before the game turns the force win list into goal barriers
 * @details Called by ie4BtlOnGoalGuardAsm. The barrier of a challenge stayed up
 * when the lists were only emptied from ie4BtlMatchStep, which a challenge does
 * not seem to reach; this runs wherever the barriers are computed.
 */
extern "C" void ie4BtlOnGoalGuard(void) {
    int match = *reinterpret_cast<int *>(BTL_MATCH_OFFSET);

    if (match != 0) {
        applyChallengeGoals(match);
    }
}

/**
 * @brief Command a goal queues in a challenge, 0 when the challenge goes on
 * @param victory One of BTL_VICTORY_*
 * @param score Score of the match, team 0 first
 * @details Keeps the game's own rules for the original conditions: any goal of
 * team 1 loses, and a goal of team 0 only wins BTL_VICTORY_SCORE.
 */
static uint8_t challengeGoalCommand(uint8_t victory, const uint8_t *score) {
    uint32_t lead;

    switch (victory) {
    case BTL_VICTORY_LEAD_2:
        lead = 2;
        break;
    case BTL_VICTORY_LEAD_3:
        lead = 3;
        break;
    default:
        if (score[1] != 0) {
            return BTL_COMMAND_LOSE;
        }
        if (victory == BTL_VICTORY_SCORE && score[0] != 0) {
            return BTL_COMMAND_WIN;
        }
        return 0;
    }

    // Either team can get the lead first, the other one has then lost
    if (score[0] >= score[1] + lead) {
        return BTL_COMMAND_WIN;
    }
    if (score[1] >= score[0] + lead) {
        return BTL_COMMAND_LOSE;
    }
    return 0;
}

/**
 * @brief Goal handler of the challenge event table, in place of the game's at 0x003E6A44
 * @param ctx Context built by the event dispatcher
 * @param unused Event argument, not needed here
 * @param command Slot receiving the BTL_COMMAND_* to queue, NULL when the queue is full
 * @details Runs after the score has been updated. The bottom screen hides the
 * ball while a shot plays on the top screen, and only the goal sequence of a
 * real match shows it again: a challenge used to end on its first goal, so it
 * never had to. When the challenge goes on, the ball is shown here.
 */
extern "C" void ie4BtlChallengeOnGoal(const BtlEventContext *ctx, int unused, uint8_t *command) {
    const uint8_t *field = reinterpret_cast<const uint8_t *>(ctx->Field);
    uint8_t result = challengeGoalCommand(field[BTL_FIELD_VICTORY],
        reinterpret_cast<const uint8_t *>(ctx->State + BTL_STATE_SCORE));

    if (result != 0) {
        if (command != 0) {
            *command = result;
        }
        return;
    }

    int match = *reinterpret_cast<int *>(BTL_MATCH_OFFSET);

    if (match != 0) {
        btlViewHideBall(match + BTL_MATCH_VIEW, 0);
    }
}

/**
 * @brief Time-up handler of the challenge event table, in place of the game's at 0x003E6BDC
 * @param ctx Context built by the event dispatcher
 * @param unused Event argument, not needed here
 * @param command Slot receiving the BTL_COMMAND_* to queue, NULL when the queue is full
 * @details Running out of time only wins the conditions that are about holding out.
 */
extern "C" void ie4BtlChallengeOnTimeUp(const BtlEventContext *ctx, int unused, uint8_t *command) {
    uint8_t victory = reinterpret_cast<const uint8_t *>(ctx->Field)[BTL_FIELD_VICTORY];

    if (command == 0) {
        return;
    }

    *command = (victory == BTL_VICTORY_KEEP_BALL || victory == BTL_VICTORY_CLEAN_SHEET)
        ? BTL_COMMAND_WIN : BTL_COMMAND_LOSE;
}

/**
 * @brief Runs in the kickoff step of the match, once the game has picked the team that kicks off
 * @param match The match object, see BTL_MATCH_OFFSET
 * @param state The live state buffer of that match
 * @details Called by ie4BtlOnKickoffAsm, before the team forced by a script, if
 * any, is applied, so scripts keep the last word.
 */
extern "C" void ie4BtlOnKickoff(int match, int state) {
    uint8_t victory = challengeVictory(reinterpret_cast<const uint8_t *>(match + BTL_MATCH_FIELD));

    if (victory == BTL_VICTORY_CLEAN_SHEET) {
        *reinterpret_cast<uint8_t *>(state + BTL_STATE_KICKOFF_TEAM) = 1;
    }

    // A kickoff after a goal must not inherit the ball the shot hid, see ie4BtlChallengeOnGoal
    if (challengeGoesOnAfterGoal(victory)) {
        btlViewHideBall(match + BTL_MATCH_VIEW, 0);
    }
}

/**
 * @brief Stands in for the btlPlaceGoals call that sets up the battle view, at 0x003AF850
 * @param view Battle view the goal models belong to
 * @param field Field geometry kept by that view
 * @details Removes the goal of team 1 from a clean sheet challenge, where team
 * 0 only defends; ie4BtlChallengeWalls keeps the ball out of it.
 */
extern "C" void ie4BtlPlaceGoals(int view, const void *field) {
    static const float hidden[3] = { 0.0f, 0.0f, 0.0f };

    btlPlaceGoals(view, field);

    if (challengeVictory(reinterpret_cast<const uint8_t *>(field)) != BTL_VICTORY_CLEAN_SHEET) {
        return;
    }

    int goal = btlViewObject(view, BTL_VIEW_GOAL_1);

    if (goal != 0) {
        btlViewObjectSetScale(goal, hidden);
    }
}

/**
 * @brief Stands in for the btlViewLockBall call of the shot sequence, at 0x003E8808
 * @param view Battle view, see BTL_MATCH_VIEW
 * @param visible Passed on to btlViewLockBall
 * @details The shot sequence freezes the hidden ball of a challenge for good,
 * which left it invisible for the rest of a challenge that goes on after a
 * goal. Those keep the ball free, so it shows again once the goal is over.
 */
extern "C" void ie4BtlLockGoalBall(int view, int visible) {
    int match = *reinterpret_cast<int *>(BTL_MATCH_OFFSET);

    if (match != 0 && challengeGoesOnAfterGoal(challengeVictory(reinterpret_cast<const uint8_t *>(match + BTL_MATCH_FIELD)))) {
        return;
    }

    btlViewLockBall(view, visible);
}

/** @brief Height of the top edge of the walls around a challenge field, above the field centre */
#define WALL_TOP 200.0f

/** @brief Height of a wall that reaches the ground */
#define WALL_FULL_HEIGHT 210.0f

/** @brief Height of the wall above a goal mouth, it stops at the crossbar */
#define WALL_ABOVE_GOAL_HEIGHT 160.0f

/** @brief Share of its speed the ball keeps when bouncing off a wall */
#define WALL_REBOUND 0.8f

/** @brief Results of ie4BtlChallengeWalls, telling ie4BtlChallengeWallsAsm where to go on */
enum {
    CHALLENGE_WALLS_CLOSED = 0,
    CHALLENGE_WALLS_OPEN = 1,
    CHALLENGE_WALLS_BUILT = 2
};

/** @brief Adds an upright wall along the x axis, from the corner x, z of the field towards +x */
static void addGoalLineWall(int world, const float *field, float x, float z, float length, float height, float dirX) {
    btlAddWall(world, 1, 3, x, field[1] + WALL_TOP, z, length, height,
        dirX, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, WALL_REBOUND);
}

/**
 * @brief Picks the walls that close the goal lines of a challenge field
 * @param world Collision world of the ball, see BTL_MATCH_WALLS
 * @param field Field geometry of the match
 * @return uint32_t One of CHALLENGE_WALLS_*
 * @details Called by ie4BtlChallengeWallsAsm. The game only leaves the goal
 * mouths open for BTL_VICTORY_SCORE and closes both goal lines for any other
 * condition, so the ball bounced off the goal of team 0 under a lead. A lead
 * needs both mouths open; a clean sheet needs the one of team 0, at +z, open
 * and the goal line of team 1, whose goal is gone, closed.
 */
extern "C" uint32_t ie4BtlChallengeWalls(int world, const uint8_t *field) {
    uint8_t victory = field[BTL_FIELD_VICTORY];

    if (victory == BTL_VICTORY_SCORE || victory == BTL_VICTORY_LEAD_2 || victory == BTL_VICTORY_LEAD_3) {
        return CHALLENGE_WALLS_OPEN;
    }
    if (victory != BTL_VICTORY_CLEAN_SHEET) {
        return CHALLENGE_WALLS_CLOSED;
    }

    const float *geometry = reinterpret_cast<const float *>(field);
    float width = geometry[3];
    float length = geometry[5];
    float x = geometry[0];
    float z = geometry[2];
    float side = width - *reinterpret_cast<const float *>(field + BTL_FIELD_GOAL_MOUTH);

    // Goal of team 0: both sides of the mouth, then above it
    addGoalLineWall(world, geometry, x + width, z + length, side, WALL_FULL_HEIGHT, -1.0f);
    addGoalLineWall(world, geometry, x - width, z + length, side, WALL_FULL_HEIGHT, 1.0f);
    addGoalLineWall(world, geometry, x - width, z + length, 2.0f * width, WALL_ABOVE_GOAL_HEIGHT, 1.0f);

    // Goal line of team 1, from one corner to the other
    addGoalLineWall(world, geometry, x - width, z - length, 2.0f * width, WALL_FULL_HEIGHT, 1.0f);

    return CHALLENGE_WALLS_BUILT;
}

/**
 * @brief Tells whether a player of a challenge may not shoot
 * @param field Field geometry of the match, a challenge
 * @param team Team of the player
 * @return uint32_t Non-zero to forbid the shot
 * @details Called by ie4BtlShotCheckAsm in place of the game's test, which
 * forbids every shot when the challenge has no goals. A clean sheet has no
 * goal for team 0 to shoot at either.
 */
extern "C" uint32_t ie4BtlShotForbidden(const uint8_t *field, uint32_t team) {
    uint8_t victory = field[BTL_FIELD_VICTORY];

    if (victory == BTL_VICTORY_GET_BALL || victory == BTL_VICTORY_KEEP_BALL) {
        return 1;
    }

    return victory == BTL_VICTORY_CLEAN_SHEET && team == 0;
}
