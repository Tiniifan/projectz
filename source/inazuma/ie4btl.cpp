#include <stdint.h>
#include "../inazuma/ie4btl.h"

static_assert(sizeof(BtlScriptArg) == 8, "BtlScriptArg must be 8 bytes");
static_assert(sizeof(BtlTeamSetup) == 80, "BtlTeamSetup must be 80 bytes");
static_assert(__builtin_offsetof(BtlTeamSetup, CoachHash) == 16, "BtlTeamSetup layout mismatch");
static_assert(__builtin_offsetof(BtlTeamSetup, EmblemId) == 20, "BtlTeamSetup layout mismatch");
static_assert(__builtin_offsetof(BtlTeamSetup, TeamName) == 40, "BtlTeamSetup layout mismatch");

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
 * @brief Stands in for the btlMatchStepUnk call of the match update
 * @details Hooked in place of that call, so it runs once per frame of live play.
 */
extern "C" uint32_t ie4BtlMatchStep(int match) {
    uint32_t result = btlMatchStepUnk(match);

    checkPenaltyAreas(match);

    return result;
}
