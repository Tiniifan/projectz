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
 * @brief Seed value marking a save file whose payload is stored in clear
 * @details The game never writes this seed: it draws seeds until it gets a non
 * zero one, so 0 is free to use as a marker. ie4SaveEncrypt stamps it into the
 * SaveFooter and ie4SaveDecrypt takes it as "this payload is not encrypted".
 * @note An unpatched game rejects such a file outright rather than loading
 * garbage, since its own decryption fails on a null seed
 */
#define IE4_SAVE_PLAINTEXT_SEED 0

/**
 * @brief Whether this build works with save files whose payload is not encrypted
 * @details Governs both directions at once, so a build can never write files it
 * would refuse to read back:
 * - true: ie4SaveEncrypt stores the payload in clear and stamps
 *   IE4_SAVE_PLAINTEXT_SEED into the footer, and ie4SaveDecrypt takes that
 *   marker at face value and loads the payload as is
 * - false: ie4SaveEncrypt encrypts with a freshly drawn seed and ie4SaveDecrypt
 *   refuses the marker, which is exactly what the original game functions do
 * Encrypted save files load whatever this is set to, so a retail save is never
 * locked out.
 * @note Folded at compile time, so a false build emits the game's original
 * behaviour with no runtime check left
 */
const bool IE4_SAVE_ALLOW_PLAINTEXT = false;

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
 * @brief Loads a save payload in place, encrypted or not
 * @param seed Seed read from the SaveFooter of the file
 * @param data Payload to load, modified in place when it is encrypted
 * @param size Size of the payload in bytes, footer excluded
 * @return uint32_t 1 on success, 0 if the buffer is null or if the payload is in
 * clear while IE4_SAVE_ALLOW_PLAINTEXT is false
 * @details A seed of IE4_SAVE_PLAINTEXT_SEED means the payload is already in
 * clear: it is left untouched when IE4_SAVE_ALLOW_PLAINTEXT is true, and
 * rejected when it is false. Any other seed decrypts the way the game does, so
 * retail save files load whatever that constant is set to.
 * @note Replaces the game function originally located at 0x0033EFDC, which
 * instead failed outright on a null seed
 * @warning The caller is expected to have validated the CRC32 beforehand
 */
uint32_t ie4SaveDecrypt(uint32_t seed, char *data, uint32_t size);

/**
 * @brief Stores a save payload in place, encrypted or not
 * @param data Payload to store, encrypted in place unless the build keeps it clear
 * @param size Size of the payload in bytes, footer excluded
 * @return uint32_t The seed the caller has to stamp into the SaveFooter:
 * IE4_SAVE_PLAINTEXT_SEED when IE4_SAVE_ALLOW_PLAINTEXT is true, otherwise the
 * non zero seed the payload was encrypted with. 0 on failure, which both call
 * sites ignore anyway
 * @details The caller computes the CRC32 after this call, so the checksum always
 * matches what actually sits in the file.
 * @note Replaces the game function originally located at 0x0033F098
 * @warning A payload left in clear is readable by any hex editor and is rejected
 * by an unpatched game
 */
uint32_t ie4SaveEncrypt(char *data, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif // IE4SAVE_H
