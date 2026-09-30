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

/** @brief Offset in a player entry of its position, three floats */
#define BTL_PLAYER_POSITION 0

/** @brief Offset in a player entry of its CharaparamID hash, copied from BattlePlayer */
#define BTL_PLAYER_PARAM_ID 340

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