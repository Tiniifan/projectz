#ifndef IE4CHARAFLAGS_H
#define IE4CHARAFLAGS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Pointer to the module holding the CharaFlags singleton
 * @details The object itself lives at offset 16 of what this address points at,
 * see ie4CharaFlags.
 */
#define CHARA_FLAGS_HOLDER 0x5B0988

/**
 * @brief Offset of the model number inside a chara_base record
 * @details Second field of CHARA_BASE_INFO, stored as a 16 bit value. For a
 * player it doubles as the character number: its record is keyed by
 * CRC32("cp%04d") of this very field, and so are its model and parameter files.
 */
#define IE4_CHARA_BASE_MODEL_OFFSET 4

/**
 * @brief Offset of the "players collected" tally, relative to its own object
 */
#define IE4_CHARA_RECRUIT_COUNT_OFFSET 282

/**
 * @brief Value the "players collected" tally saturates at
 */
#define IE4_CHARA_RECRUIT_COUNT_MAX 9999

/**
 * @brief Identifiers of the flag arrays, as numbered by ie4CharaFlagQuery
 * @details Arrays IE4_CHARA_FLAG_ENCOUNTERED to IE4_CHARA_FLAG_UNK6 are indexed
 * by model number; the others are indexed by other id spaces and are left alone
 * by this module.
 */
#define IE4_CHARA_FLAG_UNLOCK_A    0
#define IE4_CHARA_FLAG_UNLOCK_B    1
#define IE4_CHARA_FLAG_ENCOUNTERED 2
#define IE4_CHARA_FLAG_RECRUITED   3
#define IE4_CHARA_FLAG_NEW         4
#define IE4_CHARA_FLAG_UNK5        5
#define IE4_CHARA_FLAG_UNK6        6
#define IE4_CHARA_FLAG_UNK9        9
#define IE4_CHARA_FLAG_BYTE10      10
#define IE4_CHARA_FLAG_UNK11       11
#define IE4_CHARA_FLAG_BYTE12      12

/**
 * @brief Number of model numbers the game's own flag arrays hold
 * @details Each of the five model indexed arrays is a 2048 bit field, and every
 * setter of the original game throws std::out_of_range past that, which ends in
 * _assert_func and a PANIC break. Model numbers therefore stop at 2047 in an
 * unpatched game; retail data never goes above 2035.
 */
#define IE4_CHARA_FLAG_GAME_MAX 2048

/**
 * @brief Highest model number + 1 this module keeps flags for
 * @details Model numbers from IE4_CHARA_FLAG_GAME_MAX up are stored in the
 * extension arrays of this module rather than in the game object. Chrono Stone,
 * which runs the same class sized 4096, is the reference for this bound.
 * @note Anything at or above this is simply ignored: reads answer 0 and writes
 * are dropped, so a model number of any value can no longer crash the game
 * @warning Only the game's own arrays reach game.ie4. Flags of models at or
 * above IE4_CHARA_FLAG_GAME_MAX live in the injected code's memory and are lost
 * when the game is closed, so such a player is playable and recruitable but is
 * not remembered as seen, collected or new across a reload
 */
#define IE4_CHARA_FLAG_MAX 4096

/**
 * @brief Per character progress flags, the object behind CHARA_FLAGS_HOLDER
 * @details Allocated as a single 1848 byte block (see the constructor at
 * 0x005299B0). Everything from `unlockA` to `Unk584` is written to game.ie4 in
 * one 1664 byte block, as record 0x25 type 1.
 */
typedef struct
{
    /** @brief Virtual table, holds the Save and Load methods of this object */
    void *vtable;                  // 0x000
    /** @brief 2560 flags, not indexed by model number */
    uint32_t unlockA[80];          // 0x004
    /** @brief 512 flags, not indexed by model number */
    uint32_t unlockB[16];          // 0x144
    /** @brief Characters already met, indexed by model number */
    uint32_t encountered[64];      // 0x184
    /** @brief Characters actually recruited, indexed by model number */
    uint32_t recruited[64];        // 0x284
    /** @brief Characters still carrying the "new" marker, by model number */
    uint32_t isNew[64];            // 0x384
    /** @brief 2048 flags by model number, purpose not identified */
    uint32_t Unk484[64];           // 0x484
    /** @brief 2048 flags by model number, purpose not identified */
    uint32_t Unk584[64];           // 0x584
    /** @brief 128 flags, not indexed by model number */
    uint32_t Unk684[4];            // 0x684
    /** @brief 128 byte sized values, not indexed by model number */
    uint8_t Unk694[128];           // 0x694
    /** @brief 32 flags, not indexed by model number */
    uint32_t Unk714[1];            // 0x714
    /** @brief 32 byte sized values, not indexed by model number */
    uint8_t Unk718[32];            // 0x718
} CharaFlags;                      // 0x738

/**
 * @brief Returns the progress flags object, or 0 before it is built
 */
CharaFlags *ie4CharaFlags(void);

/**
 * @brief Reads one flag
 * @param flags Progress flags object
 * @param array One of the IE4_CHARA_FLAG_* array ids
 * @param index Model number for the model indexed arrays, an id of that array's
 * own space otherwise
 * @return int 1 when the flag is set, 0 when it is clear, out of range or when
 * the array id has no flags
 * @note Model numbers at or above IE4_CHARA_FLAG_GAME_MAX are served from this
 * module's extension arrays
 */
int ie4CharaFlagGet(const CharaFlags *flags, uint32_t array, uint32_t index);

/**
 * @brief Writes one flag, dropping anything out of range instead of throwing
 * @param flags Progress flags object
 * @param array One of the IE4_CHARA_FLAG_* array ids
 * @param index Model number for the model indexed arrays
 * @param value Non zero to set the flag, 0 to clear it
 */
void ie4CharaFlagSet(CharaFlags *flags, uint32_t array, uint32_t index, int value);

/**
 * @brief Tells whether a character was already met
 * @param unused The game passes a dummy first argument here, it is not read
 * @param model Model number of the character
 * @return int 1 when met, 0 otherwise
 * @note Replaces the game function originally located at 0x001F2D30, which read
 * past the end of its array for a model number of 2048 or more
 */
int ie4CharaFlagIsEncountered(int unused, uint32_t model);

/**
 * @brief Tells whether a character was recruited
 * @param unused The game passes a dummy first argument here, it is not read
 * @param model Model number of the character
 * @return int 1 when recruited, 0 otherwise
 * @note Replaces the game function originally located at 0x001F2F00
 */
int ie4CharaFlagIsRecruited(int unused, uint32_t model);

/**
 * @brief Marks a character as met, or clears that flag
 * @param unused The game passes a dummy first argument here, it is not read
 * @param model Model number of the character
 * @param value Non zero to set the flag, 0 to clear it
 * @return int The word index the game's version returned, which no caller reads
 * @note Replaces the game function originally located at 0x001F2D70, which threw
 * std::out_of_range for a model number of 2048 or more
 */
int ie4CharaFlagSetEncountered(int unused, uint32_t model, int value);

/**
 * @brief Sets or clears the "new" marker of a character
 * @note Replaces the game function originally located at 0x001F2DF4
 */
int ie4CharaFlagSetNew(int unused, uint32_t model, int value);

/**
 * @brief Sets or clears the CharaFlags::Unk484 flag of a character
 * @note Replaces the game function originally located at 0x001F2E78
 */
int ie4CharaFlagSetUnk5(int unused, uint32_t model, int value);

/**
 * @brief Sets or clears the CharaFlags::Unk584 flag of a character
 * @note Replaces the game function originally located at 0x001F2F40
 */
int ie4CharaFlagSetUnk6(int unused, uint32_t model, int value);

/**
 * @brief Reads any flag array into the small variant the scripts consume
 * @param out Receives a type tag at offset 0, 3 for a flag and 1 for a value,
 * and the result itself at offset 4
 * @param flags Progress flags object
 * @param array One of the IE4_CHARA_FLAG_* array ids
 * @param index Model number, or an id of that array's own space
 * @return void* `out`
 * @note Replaces the game function originally located at 0x002D6608, which read
 * out of bounds for any index past the end of the array it was given
 */
void *ie4CharaFlagQuery(void *out, CharaFlags *flags, uint32_t array, uint32_t index);

/**
 * @brief Records a character as met and recruited, updating the collection tally
 * @param flags Progress flags object
 * @param baseRecord chara_base record of the character being recruited
 * @param counterObject Object holding the "players collected" tally
 * @details Replaces the flag block the recruit routine had inlined at
 * 0x0039B900 to 0x0039BA1C, which threw std::out_of_range three times over for a
 * model number of 2048 or more. A character met for the first time gets the
 * "new" marker, and the tally only counts him the first time he actually joins.
 */
void ie4CharaMarkRecruited(CharaFlags *flags, const void *baseRecord, void *counterObject);

/**
 * @brief Sets or clears the "new" marker, for the script command that toggles it
 * @param flags Progress flags object
 * @param model Model number of the character
 * @param value Non zero to set the marker, 0 to clear it
 * @note Replaces the tail the command at 0x0028EED0 had inlined from 0x0028EF5C
 */
void ie4CharaFlagMarkNew(CharaFlags *flags, uint32_t model, int value);

#ifdef __cplusplus
}
#endif

#endif // IE4CHARAFLAGS_H
