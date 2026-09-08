#ifndef IE4SAVE_H
#define IE4SAVE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Name of the save file holding the full game state
 * @details Contains items, players, formations, ... Always IE4_SAVE_GAME_SIZE bytes.
 */
#define IE4_SAVE_GAME_NAME "game.ie4"

/**
 * @brief Name of the save file holding the save slot summary
 * @details Contains the first five players, the player name, the team name,
 * the level, the chapter and the play time. Always IE4_SAVE_HEAD_SIZE bytes.
 */
#define IE4_SAVE_HEAD_NAME "head.ie4"

/**
 * @brief Total size in bytes of game.ie4, footer included
 */
#define IE4_SAVE_GAME_SIZE 0xC600

/**
 * @brief Total size in bytes of head.ie4, footer included
 */
#define IE4_SAVE_HEAD_SIZE 0x100

/**
 * @brief Size in bytes of the SaveFooter appended to every .ie4 file
 */
#define IE4_SAVE_FOOTER_SIZE 8

/**
 * @brief Number of shuffle rounds applied when building a SaveCipher table
 */
#define IE4_SAVE_SHUFFLE_ROUNDS 0x1000

/**
 * @brief Trailer stored in the last 8 bytes of every .ie4 file
 * @details Laid out little-endian right after the encrypted payload, so a save
 * file is `[ payload : size - 8 bytes ][ SaveFooter : 8 bytes ]`.
 * @note The checksum covers the *encrypted* payload, not the plaintext: on write
 * the payload is encrypted first and only then checksummed.
 */
typedef struct
{
    /** @brief CRC32 of the encrypted payload (the first size - 8 bytes of the file) */
    uint32_t crc32;
    /** @brief Seed the payload was encrypted with, needed to rebuild the cipher table */
    uint32_t seed;
} SaveFooter;

/**
 * @brief State of the substitution cipher protecting .ie4 save files
 * @details The layout of the first 260 bytes matches the context the game builds
 * on the stack: the seed at offset 0 and the 256 byte substitution table at
 * offset 4. The xorshift state is kept locally here, whereas the game drives its
 * global RNG instead (see the @note on saveCipherInit).
 */
typedef struct
{
    /** @brief Seed the table was generated from, never 0 for a valid context */
    uint32_t seed;
    /** @brief Substitution table, a permutation of 0..255 derived from the seed */
    uint8_t table[256];
    /** @brief Internal xorshift128 state used while generating the table */
    uint32_t states[4];
} SaveCipher;

/**
 * @brief Resets the xorshift128 state of a cipher context from a seed
 * @param cipher Cipher context whose RNG state is initialised
 * @param seed Seed value, or 0 to keep the default constant state
 * @note A seed of 0 leaves the four constants 0x6C078966, 0xDD5254A5,
 * 0xB9523B81 and 0x03DF95B3 in place; any other seed derives the first three
 * states from it while the fourth always keeps 0x03DF95B3.
 * @warning This only touches the RNG, it does not rebuild the substitution table
 */
void saveCipherSeedRng(SaveCipher *cipher, uint32_t seed);

/**
 * @brief Advances the xorshift128 state of a cipher context by one step
 * @param cipher Cipher context to advance
 * @param modulo Upper bound of the returned value, or 0 for the raw 32 bit state
 * @return uint32_t The new state, reduced modulo `modulo` when it is non zero
 */
uint32_t saveCipherNextRandom(SaveCipher *cipher, uint32_t modulo);

/**
 * @brief Builds the substitution table of a cipher context from a seed
 * @param cipher Cipher context to initialise
 * @param seed Seed the table is derived from, must not be 0
 * @details Fills the table with the identity permutation, then applies
 * IE4_SAVE_SHUFFLE_ROUNDS swaps driven by the seeded xorshift.
 * @note The game generates this table with its *global* RNG, saving and
 * restoring it around the call. This implementation keeps the RNG inside the
 * context instead, which produces the exact same table without perturbing the
 * game's global RNG stream.
 * @warning A seed of 0 yields a table that cannot decrypt anything meaningful
 */
void saveCipherInit(SaveCipher *cipher, uint32_t seed);

/**
 * @brief Applies the cipher to a buffer, in place
 * @param cipher Cipher context whose table was built by saveCipherInit
 * @param data Buffer to transform, modified in place
 * @param size Number of bytes to transform
 * @note The transform is a plain XOR and is therefore its own inverse: the same
 * call encrypts a plaintext buffer and decrypts a ciphertext buffer.
 * @warning `data` must hold at least `size` bytes
 */
void saveCipherProcess(const SaveCipher *cipher, uint8_t *data, uint32_t size);

/**
 * @brief Decrypts a save payload in place with a known seed
 * @param seed Seed read from the SaveFooter of the file
 * @param data Payload to decrypt, modified in place
 * @param size Size of the payload in bytes, footer excluded
 * @return uint32_t 1 on success, 0 if the seed is 0 or the buffer is null
 * @note Replaces the game function originally located at 0x0033EFDC
 * @warning The caller is expected to have validated the CRC32 beforehand
 */
uint32_t ie4SaveDecrypt(uint32_t seed, char *data, uint32_t size);

/**
 * @brief Encrypts a save payload in place with a freshly drawn seed
 * @param data Payload to encrypt, modified in place
 * @param size Size of the payload in bytes, footer excluded
 * @return uint32_t The non zero seed the payload was encrypted with, or 0 on failure
 * @note Replaces the game function originally located at 0x0033F098
 * @warning The returned seed must be written to the SaveFooter, otherwise the
 * payload becomes unrecoverable
 */
uint32_t ie4SaveEncrypt(char *data, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif // IE4SAVE_H
