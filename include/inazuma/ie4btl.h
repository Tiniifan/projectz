#ifndef IE4BTL_H
#define IE4BTL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Memory offset for battle player data
 * @details Base memory address where player battle data is stored
 */
#define BTL_PLAYER_OFFSET 0x5B0B31

/**
 * @brief Memory offset for rival party data
 * @details Base memory address where rival team battle data is stored
 */
#define BTL_RIVAL_PARTY_OFFSET 0x5B0968

/**
 * @brief Memory offset for btl chara handle data
 * @details Base memory address where model (chara handle) is used during a soccer battle
 */
#define BTLCHARA_Handle_OFFSET 0x5B0998

/**
 * @brief Memory offset for battle team manager data
 * @details Base memory address where team management structures (player slots, player counts) are stored during a soccer battle
 */
#define BTL_TEAM_MANAGER_OFFSET 0x5B0990

/**
 * @brief Retrieves battle player data offset based on team and hash
 * @param basePtr Base pointer to memory location
 * @param team Team identifier (0 = player team, 1 = rival team)
 * @param hash Hash value for player identification
 * @return uint32_t* Pointer to the battle player data offset
 * @note Returns NULL if player not found or invalid parameters
 */
uint32_t* getBtlPlayerOffset(int* basePtr, int team, uint32_t hash);

/**
 * @brief Retrieves btl chara handle data offset based on id of the character
 * @param base Base (this memory address is already assigned to the team)
 * @param hash Hash value for player identification
 * @param unkArgument Unknown argument, but it's always 0
 * @return uint32_t* Pointer to the btl chara handle data offset
 * @note Returns 0 if player not found or invalid parameters
 */
int getBtlCharaHandleOffsetByID(int base, uint32_t hash, int unkArgument);

/**
 * @brief Retrieves btl chara handle data offset based on the number of the character
 * @param base Base (this memory address is already assigned to the team)
 * @param playerNumber The position of the player in the formation (like 0 = the goalkeeper)
 * @return uint32_t* Pointer to the btl chara handle data offset
 * @note Returns 0 if player not found or invalid parameters
 */
int getBtlCharaHandleOffsetByNumber(int base, int playerNumber);

/**
 * @brief Pointer to the match object
 * @details Allocated once at boot and reused by every match, so it is never
 * cleared between two of them. All BTL_MATCH_* offsets are relative to it.
 */
#define BTL_MATCH_OFFSET 0x5B0920

/** @brief Index (0 or 1) of the live state buffer, the match keeps two of them */
#define BTL_MATCH_STATE_INDEX 56052

/** @brief Offset of the first state buffer */
#define BTL_MATCH_STATE_BASE 56072

/** @brief Size of one state buffer */
#define BTL_MATCH_STATE_SIZE 17368

/**
 * @brief Offset of the field geometry
 * @details Floats: centre x, y, z at 0, half extents x, y, z at 12. The half
 * width and half depth of a penalty area are at 48 and 56.
 */
#define BTL_MATCH_FIELD 142912

/**
 * @brief Offset of the battle script object, the one holding the CallMatch* callbacks
 * @details Its first word is the Squirrel VM, and the byte at 4 is set while
 * that VM is suspended inside a callback.
 */
#define BTL_MATCH_SCRIPT 152460

/** @brief Offset within a state buffer of the player slot holding the ball, 255 when loose (byte) */
#define BTL_STATE_BALL_HOLDER 5

/** @brief Offset of the player array within a state buffer */
#define BTL_STATE_PLAYERS 196

/** @brief Size of one entry of the player array */
#define BTL_STATE_PLAYER_SIZE 432

/** @brief Number of entries of the player array, both teams included */
#define BTL_STATE_PLAYER_COUNT 32

/** @brief Offset of the team array within a state buffer */
#define BTL_STATE_TEAMS 14972

/** @brief Size of one entry of the team array */
#define BTL_STATE_TEAM_SIZE 1044

/** @brief Offset in a team entry of the direction it attacks along z, +1 or -1 (signed byte) */
#define BTL_TEAM_DIRECTION 40

/** @brief Offset in a team entry of its number of players on the field */
#define BTL_TEAM_FIELD_COUNT 41

/** @brief Offset in a team entry of the slot of its first player in the player array */
#define BTL_TEAM_FIRST_SLOT 43

/** @brief Offset in a team entry of the index of its goalkeeper among its players, 255 for none */
#define BTL_TEAM_KEEPER 44

/** @brief Offset in a team entry of its number of formations, see BTL_TEAM_FORMATIONS (byte) */
#define BTL_TEAM_FORMATION_COUNT 47

/**
 * @brief Offset in a team entry of its formations, the home positions of its players
 * @details Up to BTL_TEAM_FORMATION_MAX entries of BTL_TEAM_FORMATION_SIZE
 * bytes, each holding two floats, x then z, per player of the team, in the
 * order of its players.
 */
#define BTL_TEAM_FORMATIONS 52

/** @brief Size of one formation, see BTL_TEAM_FORMATIONS */
#define BTL_TEAM_FORMATION_SIZE 92

/** @brief Maximum number of formations of a team */
#define BTL_TEAM_FORMATION_MAX 10

/**
 * @brief Offset in a team entry of the roles of its players
 * @details BTL_TEAM_ROLE_SIZE bytes per player, in the order of its players: the
 * role at 0 (16 bit, see BTL_ROLE_*), its flag at 2 (16 bit, see
 * BTL_ROLE_FLAG_*) and an index at 4 (byte).
 */
#define BTL_TEAM_ROLES 972

/** @brief Size of the role of one player, see BTL_TEAM_ROLES */
#define BTL_TEAM_ROLE_SIZE 6

/** @brief Role of a goalkeeper */
#define BTL_ROLE_KEEPER   1

/** @brief Role of a defender, 2 and 3 both are */
#define BTL_ROLE_DEFENDER 2

/** @brief Role flag of a goalkeeper */
#define BTL_ROLE_FLAG_KEEPER     0x10

/** @brief Role flag of a defender */
#define BTL_ROLE_FLAG_DEFENDER   0x20

/** @brief Role flag of a midfielder, roles 4 to 6 */
#define BTL_ROLE_FLAG_MIDFIELDER 0x40

/** @brief Role flag of a forward, roles 7 and above */
#define BTL_ROLE_FLAG_FORWARD    0x80

/**
 * @brief Offset of the "force win" list, filled by CMND_BTL_SET_FORCE_WIN
 * @details BTL_FORCE_LIST_SIZE BtlForceEntry. A player listed here wins every
 * duel, and a listed goalkeeper also closes his goal with a barrier the ball
 * bounces off, see BTL_MATCH_GOAL_GUARD.
 */
#define BTL_MATCH_FORCE_WIN 152524

/** @brief Offset of the "force lose" list, filled by CMND_BTL_SET_FORCE_LOSE, same layout */
#define BTL_MATCH_FORCE_LOSE 152780

/** @brief Number of entries of a force list */
#define BTL_FORCE_LIST_SIZE 32

/**
 * @brief Offset of the two goal barrier flags, one byte per team (team 0 first)
 * @details Recomputed every frame from the goalkeepers found in the force win list.
 */
#define BTL_MATCH_GOAL_GUARD 152516

/**
 * @brief One entry of a force list
 * @details Empty when Team is -1 and Id is 0; the lookup stops at the first
 * empty entry.
 */
typedef struct {
    /** @brief CharaparamID hash when Team is -1, else the index of the player within Team */
    uint32_t Id;

    /** @brief Team of the player, or -1 to look it up by CharaparamID */
    int16_t Team;

    /** @brief Unknown, every writer of a script stores 0 */
    uint16_t Unk6;
} BtlForceEntry;

/**
 * @brief Offset of the battle view, the 3D scene drawn on the bottom screen
 * @details Inside the display copy of the match that starts at match + 153564.
 */
#define BTL_MATCH_VIEW 153568

/**
 * @brief Offset of the collision world the ball bounces off, see btlAddWall
 * @details Up to 32 entries of 88 bytes, their count is the word at 2816. The
 * field setup fills it with the walls around a challenge field and the goal posts.
 */
#define BTL_MATCH_WALLS 149608

/** @brief Index of the goal model of team 0 among the battle view objects, see btlViewObject */
#define BTL_VIEW_GOAL_0 0

/** @brief Index of the goal model of team 1 among the battle view objects */
#define BTL_VIEW_GOAL_1 1

/** @brief Offset in a player entry of its position, three floats */
#define BTL_PLAYER_POSITION 0

/** @brief Offset in a player entry of its CharaparamID hash, copied from BattlePlayer */
#define BTL_PLAYER_PARAM_ID 340

/**
 * @brief Offset within a state buffer of the score, one byte per team
 * @details Team 0 at this offset, team 1 right after it.
 */
#define BTL_STATE_SCORE 14792

/**
 * @brief Offset within a state buffer of the team that kicks off, 0 or 1 (byte)
 * @details Picked again by the kickoff step of the match, see ie4BtlOnKickoff.
 */
#define BTL_STATE_KICKOFF_TEAM 14797

/**
 * @brief Offset in the field geometry of the kind of game being played (16 bit)
 * @details 1 for a match, 2 for anything else. The challenge event table only
 * reacts to the match events while this is BTL_FIELD_KIND_CHALLENGE.
 */
#define BTL_FIELD_KIND 100

#define BTL_FIELD_KIND_MATCH     1
#define BTL_FIELD_KIND_CHALLENGE 2

/** @brief Offset in the field geometry of the victory condition, one of BTL_VICTORY_* (byte) */
#define BTL_FIELD_VICTORY 102

/**
 * @brief Offset in the field geometry of the rule flags, see BTL_RULE_* (byte)
 * @details Copied from the soccer_config entry of the match when it is set up.
 */
#define BTL_FIELD_RULES 103

/** @brief Offset in the field geometry of the half width of a goal mouth, FieldGoalMouse_X (float) */
#define BTL_FIELD_GOAL_MOUTH 84

/** @brief Rule flag: the match is played in two halves, always set */
#define BTL_RULE_TWO_HALVES 0x02

/** @brief Rule flag: a draw at full time goes to a penalty shoot-out, after extra time if any */
#define BTL_RULE_PENALTIES  0x04

/** @brief Rule flag: a draw at full time goes to extra time */
#define BTL_RULE_EXTRA_TIME 0x08

/**
 * @brief Victory conditions of a soccer_config entry
 * @details The game only knows 0 to 4, the others are handled by this module.
 * A challenge always ends as soon as either team meets the condition.
 */
/** @brief Regular match, decided by the score */
#define BTL_VICTORY_NONE        0
/** @brief Get the ball back; the other team kicks off */
#define BTL_VICTORY_GET_BALL    1
/** @brief Score a goal */
#define BTL_VICTORY_SCORE       2
/** @brief Keep the ball until the time runs out, which is always 5 */
#define BTL_VICTORY_KEEP_BALL   3
/** @brief Replaced at load by GET_BALL, SCORE or KEEP_BALL, 20%/60%/20% */
#define BTL_VICTORY_RANDOM      4
/** @brief Lead by two goals */
#define BTL_VICTORY_LEAD_2      5
/** @brief Lead by three goals */
#define BTL_VICTORY_LEAD_3      6
/** @brief Concede no goal until the time runs out; the other team kicks off and has no goal */
#define BTL_VICTORY_CLEAN_SHEET 7

/**
 * @brief Match commands a challenge event handler can queue
 * @details The handler writes one in the first byte of the slot it is given.
 */
/** @brief Ends the match, won by team 0 */
#define BTL_COMMAND_WIN  1
/** @brief Ends the match, won by team 1 */
#define BTL_COMMAND_LOSE 2

/**
 * @brief Context handed to the handlers of the match event tables
 * @details Built on the stack by the event dispatcher at 0x003BAA28 for every
 * handler it calls.
 */
typedef struct {
    /** @brief Match + 152444, its word at 16 is the battle script object */
    int ScriptHolder;

    /** @brief Match + 4, purpose not identified */
    int Unk4;

    /** @brief Match + 28, details of the event being raised */
    int Event;

    /** @brief The live state buffer of the match */
    int State;

    /** @brief The field geometry of the match, see BTL_MATCH_FIELD */
    int Field;
} BtlEventContext;

/**
 * @brief Flags of a SoccerInfo
 */
/** @brief A draw at full time goes to extra time (Unk3 of SOCCER_INFO) */
#define SOCCER_INFO_EXTRA_TIME 0x01
/** @brief A draw at full time goes to a penalty shoot-out (Unk4 of SOCCER_INFO) */
#define SOCCER_INFO_PENALTIES  0x02

/**
 * @brief One SOCCER_INFO entry of data/res/soccer/soccer_config.cfg.bin, as the game keeps it
 * @details Built by the row handler at 0x003A80FC and looked up by SoccerId.
 * The cfg columns are, in order: SoccerId, TeamParamId, Mode, extra time
 * (Unk3), penalties (Unk4), Bgm, VictoryCondition, ScriptId, Time, WinFlag
 * (NextScript). The match loader copies Mode, VictoryCondition, Time and the
 * flags into the match rules, and from there into the field geometry.
 */
typedef struct {
    /** @brief CRC32 of "btl%04d" */
    uint32_t SoccerId;

    /** @brief TEAM_PARAM_INFO entry of the other team */
    uint32_t TeamParamId;

    /** @brief Music played during the match, "M_xx"; when empty, M_78 for a challenge and M_08 otherwise */
    const char *Bgm;

    /** @brief Script loaded for the match */
    uint16_t ScriptId;

    /**
     * @brief Index of the CharaFlags unlockA flag raised when the match is won, 0 for none
     * @details Set by 0x0040CFA0 along with the Unk5 flag of every player of the other team.
     */
    uint16_t WinFlag;

    /** @brief 1 for a 5 vs 5 challenge, 2 for an 11 vs 11 match with 5 substitutes */
    uint8_t Mode;

    /** @brief One of BTL_VICTORY_* */
    uint8_t VictoryCondition;

    /**
     * @brief Length of the game, scaled by 60 into frames
     * @details 0 picks 45 for a match and 15 otherwise. Ignored by KEEP_BALL, which always uses 5.
     */
    uint8_t Time;

    /** @brief SOCCER_INFO_* flags */
    uint8_t Flags;
} SoccerInfo;

/**
 * @brief Type tags of a BtlScriptArg
 */
#define BTL_SCRIPT_ARG_INT    1
#define BTL_SCRIPT_ARG_FLOAT  2
#define BTL_SCRIPT_ARG_BOOL   3
#define BTL_SCRIPT_ARG_STRING 4
#define BTL_SCRIPT_ARG_PTR    5

/**
 * @brief One argument handed to a script callback by callBtlScriptFunc
 */
typedef struct {
    /** @brief One of the BTL_SCRIPT_ARG_* tags */
    uint8_t Type;

    /** @brief Padding */
    uint8_t Pad[3];

    /** @brief The value, reinterpreted according to Type */
    uint32_t Value;
} BtlScriptArg;

/**
 * @brief Setup of a team for the current match
 * @details Two of them sit side by side in the battle team manager and are
 * rebuilt at the start of every match, from the save for the player's team and
 * from the match data for the other one. Nothing here is written back to the save.
 */
typedef struct {
    /** @brief Uniform, 1000 * uniform number, + 2 for the away kit */
    uint32_t Uniform;

    /** @brief Unknown data field */
    uint32_t Unk4;

    /** @brief Unknown data field */
    uint32_t Unk8;

    /** @brief CRC32 of the formation name, "ifa%04d" */
    uint32_t FormationHash;

    /** @brief CRC32 of the coach name, "iim%04d", 0 for no coach */
    uint32_t CoachHash;

    /** @brief Emblem number, the XXXX of the emb/ems/emf files (0 reads as 1) */
    uint16_t EmblemId;

    /** @brief Team name number, the XXXX of the ts%04d text */
    uint16_t TeamNameId;

    /** @brief Unknown, every writer of Level stores the same value here too */
    uint8_t Unk24;

    /** @brief Team level shown on the versus screen before kick-off (0-255) */
    uint8_t Level;

    /** @brief Unknown value the game derives from Level through a table, 999 if missing */
    uint16_t Unk26;

    /** @brief Unknown 12-byte data block */
    uint8_t UnkBlock1[12];

    /** @brief Custom team name, used instead of TeamNameId when not empty */
    char TeamName[40];
} BtlTeamSetup;

/**
 * @brief Retrieves the setup of a team for the current match
 * @param unused Ignored, the game passes BTL_PLAYER_OFFSET
 * @param team Team identifier (0 = player team, 1 = rival team)
 * @return BtlTeamSetup* The setup, or NULL if team is out of range or no match data exists
 */
BtlTeamSetup* getBtlTeamSetup(void* unused, uint32_t team);

/**
 * @brief Retrieves the team the player controls in the current match
 * @param unused Ignored, the game passes BTL_PLAYER_OFFSET
 * @return uint32_t Team identifier, normally 0
 */
uint32_t getBtlUserTeam(void* unused);

/**
 * @brief Recomputes the in-match stats of every player of both teams
 * @param match The match object, see BTL_MATCH_OFFSET
 * @param state The live state buffer of that match
 * @param unk Unknown argument, but it's always 0
 * @return uint32_t Unused by every caller
 * @details Starts again from the base stats and reapplies the team effects and
 * the coach bonus, so calling it any number of times is harmless. Current FP
 * and TP are left alone.
 */
uint32_t btlRecalcPlayerParams(int match, int state, int unk);

/**
 * @brief Tells whether a position lies inside one of the two penalty areas
 * @param field The field geometry, see BTL_MATCH_FIELD
 * @param position Three floats, a player entry can be passed as is
 * @param side Which area: the one a team defends is side = +direction, the one
 * it attacks is side = -direction (see BTL_TEAM_DIRECTION)
 * @param margin Grows the area by this much on every side, the game mostly passes 0
 * @return int Non-zero if inside
 */
int btlIsInPenaltyArea(const float* field, const void* position, float side, float margin);

/**
 * @brief Unidentified step of the match update, run once per frame of live play
 * @param match The match object, see BTL_MATCH_OFFSET
 * @return uint32_t Unused by its caller
 * @note Only called from the match update, which ie4BtlMatchStep hooks
 */
uint32_t btlMatchStepUnk(int match);

/**
 * @brief Places the two goal models of the match
 * @param view Battle view the models belong to
 * @param field Field geometry of the match, the battle view keeps its own copy
 * @details Called from 0x003AF838 when the battle view is set up. Shrinks both
 * goals to nothing for BTL_VICTORY_GET_BALL and BTL_VICTORY_KEEP_BALL.
 */
void btlPlaceGoals(int view, const void* field);

/**
 * @brief Adds a flat rectangular wall to the collision world of the ball
 * @param world Collision world, see BTL_MATCH_WALLS
 * @param unk84 Unknown, stored at 84 in the entry; the field walls pass 1
 * @param unk1 Unknown, stored at 1 in the entry; the field walls pass 3
 * @param x X of one corner of the wall
 * @param y Y of that corner
 * @param z Z of that corner
 * @param length Extent of the wall along dirX, dirY, dirZ
 * @param height Extent of the wall along downX, downY, downZ
 * @param dirX X of the unit vector the length runs along
 * @param dirY Y of that vector
 * @param dirZ Z of that vector
 * @param downX X of the unit vector the height runs along
 * @param downY Y of that vector
 * @param downZ Z of that vector
 * @param rebound Share of its speed the ball keeps when bouncing, the field walls use 0.8
 * @return int Non-zero once added, 0 when the world is full
 * @details The field setup at 0x003C9858 walls a challenge field in with it:
 * both touchlines, then for BTL_VICTORY_SCORE only both sides of each goal
 * mouth and the space above it, and for every other condition each goal line
 * from one corner to the other, goal mouth included.
 */
int btlAddWall(int world, uint8_t unk84, uint8_t unk1, float x, float y, float z, float length, float height,
    float dirX, float dirY, float dirZ, float downX, float downY, float downZ, float rebound);

/**
 * @brief Hides or shows the ball and its shadow on the bottom screen
 * @param view Battle view, see BTL_MATCH_VIEW
 * @param hidden Non-zero to hide them
 * @details Only records the request, the view applies it on its next frame
 * and keeps it until the next call. Shots hide the ball while they play on
 * the top screen.
 */
void btlViewHideBall(int view, int hidden);

/**
 * @brief Freezes the ball and its shadow of the bottom screen as they are
 * @param view Battle view, see BTL_MATCH_VIEW
 * @param visible Passed on to the objects, but they ignore it once frozen
 * @details Nothing unfreezes them before the match ends. The shot sequence
 * calls it in a challenge only, once the ball has been hidden, since the
 * game ends a challenge on its first goal.
 */
void btlViewLockBall(int view, int visible);

/**
 * @brief Retrieves a 3D object of the battle view
 * @param view Battle view
 * @param index Index of the object, e.g. BTL_VIEW_GOAL_0
 * @return int The object, 0 if missing
 */
int btlViewObject(int view, uint32_t index);

/**
 * @brief Sets the scale of a battle view object
 * @param object Object returned by btlViewObject
 * @param scale Three floats, all at 0 to make it disappear
 */
void btlViewObjectSetScale(int object, const float* scale);

/**
 * @brief Calls a function of a script object, without letting it suspend
 * @param script Script object, see BTL_MATCH_SCRIPT
 * @param name Name of the function to call
 * @param args Arguments to push
 * @param count Number of entries in args
 * @return int Negative if the script has no such function
 * @warning The called function must not wait, the VM stack is restored as soon
 * as it returns
 */
int callBtlScriptFunc(int* script, const char* name, const BtlScriptArg* args, int count);

/**
 * @brief Recomputes the stats of both teams of the current match
 * @details Picks up a change of coach. Does nothing outside of a match.
 */
void ie4BtlRefreshPlayerParams(void);

/**
 * @brief Structure representing a battle player with all combat-related data
 * @details Contains complete player information including stats, special moves,
 * and battle-specific attributes used during combat encounters
 */
typedef struct {
    // Identifiers
    /** @brief Character parameter ID for referencing character data */
    int32_t CharaparamID;
    
    /** @brief Base character ID for the player */
    int32_t CharaBaseID;
    
    /** @brief Description text ID for player information */
    int32_t DescriptionID;
    
    /** @brief Fighting spirit technique ID */
    int32_t FightingSpiritID;
    
    /** @brief Fighting spirit match condition ID */
    int32_t FightingSpiritMatchID;
   
    // Base stats
    /** @brief Player's elemental attribute (Fire, Wood, Earth, Air) */
    uint8_t Element;
    
    /** @brief Field position (GK, DF, MF, FW) */
    uint8_t Position;
    
    /** @brief Base Fire Points for special moves */
    uint8_t FPBase;
    
    /** @brief Base Technique Points for special moves */
    uint8_t TPBase;
    
    /** @brief Base kicking power stat */
    uint8_t KickBase;
    
    /** @brief Base dribbling skill stat */
    uint8_t DribbleBase;
    
    /** @brief Base technique skill stat */
    uint8_t TechniqueBase;
    
    /** @brief Base blocking/defense stat */
    uint8_t BlockBase;
    
    /** @brief Base speed/agility stat */
    uint8_t SpeedBase;
    
    /** @brief Base stamina/endurance stat */
    uint8_t StaminaBase;
    
    /** @brief Base catching ability stat (for goalkeepers) */
    uint8_t CatchBase;
    
    /** @brief Base luck stat affecting critical hits */
    uint8_t LuckBase;
   
    // Growth stats
    /** @brief FP growth rate per level */
    uint8_t FPGrow;
    
    /** @brief TP growth rate per level */
    uint8_t TPGrow;
    
    /** @brief Kick stat growth rate per level */
    uint8_t KickGrow;
    
    /** @brief Dribble stat growth rate per level */
    uint8_t DribbleGrow;
    
    /** @brief Technique stat growth rate per level */
    uint8_t TechniqueGrow;
    
    /** @brief Block stat growth rate per level */
    uint8_t BlockGrow;
    
    /** @brief Speed stat growth rate per level */
    uint8_t SpeedGrow;
    
    /** @brief Stamina stat growth rate per level */
    uint8_t StaminaGrow;
    
    /** @brief Catch stat growth rate per level */
    uint8_t CatchGrow;
    
    /** @brief Luck stat growth rate per level */
    uint8_t LuckGrow;
   
    // Special move data
    /** @brief Memory offset to special move data array */
    uint16_t SpecialMoveOffset;
    
    /** @brief Freedom points affecting AI behavior */
    uint8_t Freedom;
    
    /** @brief Number of special moves the player knows */
    uint8_t SpecialMoveCount;
    
    /** @brief Player group classification for team organization */
    uint8_t PlayerGroup;
    
    /** @brief Experience points growth multiplier */
    uint8_t ExperienceGrow;
    
    /** @brief Statistical variance for random stat calculations */
    uint8_t StatVariance;
    
    /** @brief Unknown data field */
    uint8_t Unk2;
    
    /** @brief Unknown 6-byte data block for future use */
    uint8_t UnkBlock1[6];
   
    // Training and team info
    /** @brief Unique training identifier */
    uint16_t TrainingUID;
    
    /** @brief Team side indicator (0 = player team, 1 = rival team) */
    uint16_t TeamSide;
    
    /** @brief Player's index ID within the team roster */
    int32_t PlayerIndexId;
   
    // Special move IDs
    /** @brief ID of the first special move */
    int32_t SpecialMove1ID;
    
    /** @brief ID of the second special move */
    int32_t SpecialMove2ID;
    
    /** @brief ID of the third special move */
    int32_t SpecialMove3ID;
    
    /** @brief ID of the fourth special move */
    int32_t SpecialMove4ID;
    
    /** @brief ID of the fifth special move */
    int32_t SpecialMove5ID;
    
    /** @brief ID of the sixth special move */
    int32_t SpecialMove6ID;
   
    // Special move levels
    /** @brief Level/power of the first special move */
    uint8_t SpecialMove1Level;
    
    /** @brief Level/power of the second special move */
    uint8_t SpecialMove2Level;
    
    /** @brief Level/power of the third special move */
    uint8_t SpecialMove3Level;
    
    /** @brief Level/power of the fourth special move */
    uint8_t SpecialMove4Level;
    
    /** @brief Level/power of the fifth special move */
    uint8_t SpecialMove5Level;
    
    /** @brief Level/power of the sixth special move */
    uint8_t SpecialMove6Level;
   
    // Level and jersey number
    /** @brief Player's current experience level */
    uint8_t Level;
    
    /** @brief Jersey number displayed on uniform */
    uint8_t PlayerNumber;
   
    // Fighting spirit
    /** @brief Available fighting spirit technique ID */
    int32_t FightingSpiritAvailableID;
    
    /** @brief Current level of fighting spirit mastery */
    uint16_t FightingSpiritLevel;
   
    // Current points
    /** @brief Current Fire Points available for special moves */
    uint16_t FPCurrent;
    
    /** @brief Current Technique Points available for special moves */
    uint16_t TPCurrent;
    
    /** @brief Maximum Fire Points capacity */
    uint16_t FPMax;
    
    /** @brief Maximum Technique Points capacity */
    uint16_t TPMax;
   
    // Final stats (after calculation)
    /** @brief Final calculated kick power (base + growth + bonuses) */
    uint16_t KickFinal;
    
    /** @brief Final calculated dribble skill (base + growth + bonuses) */
    uint16_t DribbleFinal;
    
    /** @brief Final calculated technique skill (base + growth + bonuses) */
    uint16_t TechniqueFinal;
    
    /** @brief Final calculated block skill (base + growth + bonuses) */
    uint16_t BlockFinal;
    
    /** @brief Final calculated speed (base + growth + bonuses) */
    uint16_t SpeedFinal;
    
    /** @brief Final calculated stamina (base + growth + bonuses) */
    uint16_t StaminaFinal;
    
    /** @brief Final calculated catch ability (base + growth + bonuses) */
    uint16_t CatchFinal;
    
    /** @brief Final calculated luck (base + growth + bonuses) */
    uint16_t LuckFinal;
   
    /** @brief Unknown 38-byte data block reserved for future expansion */
    uint8_t UnkBlock2[38];
} BattlePlayer;

#ifdef __cplusplus
}
#endif

#endif // IE4BTL_H