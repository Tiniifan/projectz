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
 * @brief Retrieves battle player data offset based on team and hash
 * @param basePtr Base pointer to memory location
 * @param team Team identifier (0 = player team, 1 = rival team)
 * @param hash Hash value for player identification
 * @return uint32_t* Pointer to the battle player data offset
 * @note Returns NULL if player not found or invalid parameters
 */
uint32_t* getBtlPlayerOffset(int* basePtr, int team, uint32_t hash);

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