#include <stdint.h>
#include "../inazuma/ie4save.h"
#include "../inazuma/ie4std.h"

/**
 * @brief The 256 first odd prime numbers
 * @details Reimplementation of the table the game keeps at 0x005867AC. Indexed
 * by a byte taken from the substitution table, it picks the stride used to walk
 * that table over a 256 byte block.
 */
static const uint16_t ODD_PRIMES[256] = {
       3,    5,    7,   11,   13,   17,   19,   23,   29,   31,   37,   41,   43,   47,   53,   59,
      61,   67,   71,   73,   79,   83,   89,   97,  101,  103,  107,  109,  113,  127,  131,  137,
     139,  149,  151,  157,  163,  167,  173,  179,  181,  191,  193,  197,  199,  211,  223,  227,
     229,  233,  239,  241,  251,  257,  263,  269,  271,  277,  281,  283,  293,  307,  311,  313,
     317,  331,  337,  347,  349,  353,  359,  367,  373,  379,  383,  389,  397,  401,  409,  419,
     421,  431,  433,  439,  443,  449,  457,  461,  463,  467,  479,  487,  491,  499,  503,  509,
     521,  523,  541,  547,  557,  563,  569,  571,  577,  587,  593,  599,  601,  607,  613,  617,
     619,  631,  641,  643,  647,  653,  659,  661,  673,  677,  683,  691,  701,  709,  719,  727,
     733,  739,  743,  751,  757,  761,  769,  773,  787,  797,  809,  811,  821,  823,  827,  829,
     839,  853,  857,  859,  863,  877,  881,  883,  887,  907,  911,  919,  929,  937,  941,  947,
     953,  967,  971,  977,  983,  991,  997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051,
    1061, 1063, 1069, 1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171,
    1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289,
    1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423, 1427,
    1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523,
    1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621
};

/** @brief Multiplier used to derive the xorshift states from a seed */
#define SEED_MULTIPLIER 0x6C078965

void saveCipherSeedRng(SaveCipher *cipher, uint32_t seed) {
    cipher->states[0] = 0x6C078966;
    cipher->states[1] = 0xDD5254A5;
    cipher->states[2] = 0xB9523B81;
    cipher->states[3] = 0x03DF95B3;

    if (seed == 0) {
        return;
    }

    seed = SEED_MULTIPLIER * (seed ^ (seed >> 30)) + 1;
    cipher->states[0] = seed;

    seed = SEED_MULTIPLIER * (seed ^ (seed >> 30)) + 2;
    cipher->states[1] = seed;

    cipher->states[2] = SEED_MULTIPLIER * (seed ^ (seed >> 30)) + 3;
}

uint32_t saveCipherNextRandom(SaveCipher *cipher, uint32_t modulo) {
    uint32_t x = cipher->states[0];
    uint32_t y = cipher->states[3];

    cipher->states[0] = cipher->states[1];
    cipher->states[1] = cipher->states[2];
    cipher->states[2] = y;

    x ^= x << 11;
    x ^= x >> 8;
    y ^= y >> 19;
    cipher->states[3] = x ^ y;

    if (modulo == 0) {
        return cipher->states[3];
    }

    return cipher->states[3] % modulo;
}

void saveCipherInit(SaveCipher *cipher, uint32_t seed) {
    uint32_t i;

    cipher->seed = seed;

    // Start from the identity permutation
    for (i = 0; i < 256; i++) {
        cipher->table[i] = (uint8_t)i;
    }

    saveCipherSeedRng(cipher, seed);

    // Shuffle it: each round draws two indices, reads the values stored there
    // and swaps the entries those values point at
    for (i = 0; i < IE4_SAVE_SHUFFLE_ROUNDS; i++) {
        uint32_t random = saveCipherNextRandom(cipher, 0x10000);
        uint8_t low = (uint8_t)random;
        uint8_t high = (uint8_t)(random >> 8);

        if (low != high) {
            uint8_t first = cipher->table[low];
            uint8_t second = cipher->table[high];
            uint8_t swap = cipher->table[first];

            cipher->table[first] = cipher->table[second];
            cipher->table[second] = swap;
        }
    }
}

void saveCipherProcess(const SaveCipher *cipher, uint8_t *data, uint32_t size) {
    uint32_t i;
    uint16_t stride = 0;

    for (i = 0; i < size; i++) {
        // The stride only changes once per 256 byte block, and is picked from
        // the table entry matching that block index
        if ((i & 0xFF) == 0) {
            stride = ODD_PRIMES[cipher->table[(i >> 8) & 0xFF]];
        }

        data[i] ^= cipher->table[(uint8_t)((i + 1) * stride)];
    }
}

uint32_t ie4SaveDecrypt(uint32_t seed, char *data, uint32_t size) {
    SaveCipher cipher;

    if (data == 0) {
        return 0;
    }

    // Saves written by this build carry the plaintext marker in their footer,
    // there is nothing to undo. Refuse them like the original function does when
    // this build does not allow plaintext. Retail saves carry a real seed and
    // always go through the cipher below
    if (seed == IE4_SAVE_PLAINTEXT_SEED) {
        return IE4_SAVE_ALLOW_PLAINTEXT ? 1 : 0;
    }

    saveCipherInit(&cipher, seed);

    lockAllocator();
    saveCipherProcess(&cipher, (uint8_t *)data, size);
    unlockAllocator();

    return 1;
}

uint32_t ie4SaveEncrypt(char *data, uint32_t size) {
    SaveCipher cipher;
    uint32_t seed;

    if (data == 0) {
        return 0;
    }

    // Leave the payload in clear and hand the marker back to the caller, which
    // writes it to the footer and only then checksums the buffer
    if (IE4_SAVE_ALLOW_PLAINTEXT) {
        return IE4_SAVE_PLAINTEXT_SEED;
    }

    // Draw a non zero seed from the game's global RNG, as the original does
    do {
        seed = (uint32_t)getRandomNumber(0);
    } while (seed == 0);

    saveCipherInit(&cipher, seed);

    lockAllocator();
    saveCipherProcess(&cipher, (uint8_t *)data, size);
    unlockAllocator();

    return seed;
}
