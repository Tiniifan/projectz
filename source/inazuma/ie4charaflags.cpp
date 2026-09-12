#include <stdint.h>
#include "../inazuma/ie4charaflags.h"

static_assert(sizeof(CharaFlags) == 0x738, "CharaFlags must be 1848 bytes");
static_assert(__builtin_offsetof(CharaFlags, encountered) == 388, "CharaFlags layout mismatch");
static_assert(__builtin_offsetof(CharaFlags, recruited) == 644, "CharaFlags layout mismatch");
static_assert(__builtin_offsetof(CharaFlags, isNew) == 900, "CharaFlags layout mismatch");
static_assert(__builtin_offsetof(CharaFlags, Unk484) == 1156, "CharaFlags layout mismatch");
static_assert(__builtin_offsetof(CharaFlags, Unk584) == 1412, "CharaFlags layout mismatch");
static_assert(__builtin_offsetof(CharaFlags, Unk684) == 1668, "CharaFlags layout mismatch");
static_assert(__builtin_offsetof(CharaFlags, Unk714) == 1812, "CharaFlags layout mismatch");
static_assert(__builtin_offsetof(CharaFlags, Unk718) == 1816, "CharaFlags layout mismatch");

/** @brief Number of model indexed arrays, IE4_CHARA_FLAG_ENCOUNTERED to IE4_CHARA_FLAG_UNK6 */
#define MODEL_ARRAY_COUNT 5

/** @brief Words needed to hold the model numbers the game cannot store itself */
#define EXTENSION_WORDS ((IE4_CHARA_FLAG_MAX - IE4_CHARA_FLAG_GAME_MAX) / 32)

/** @brief Type tags ie4CharaFlagQuery stamps into its result */
#define QUERY_TYPE_VALUE 1
#define QUERY_TYPE_FLAG  3

/**
 * @brief Flags of the model numbers the game's own arrays cannot index
 * @details One row per model indexed array, covering IE4_CHARA_FLAG_GAME_MAX up
 * to IE4_CHARA_FLAG_MAX. The linker script folds .bss into .text, so this starts
 * out zeroed in the injected blob.
 * @warning Never written to game.ie4, see the note on IE4_CHARA_FLAG_MAX
 */
static uint32_t sExtension[MODEL_ARRAY_COUNT][EXTENSION_WORDS];

CharaFlags *ie4CharaFlags(void) {
    uint32_t holder = *reinterpret_cast<uint32_t *>(CHARA_FLAGS_HOLDER);

    if (holder == 0) {
        return 0;
    }

    return *reinterpret_cast<CharaFlags **>(holder + 16);
}

/** @brief The game's own array for a model indexed id, or 0 for any other id */
static uint32_t *modelArray(CharaFlags *flags, uint32_t array) {
    switch (array) {
        case IE4_CHARA_FLAG_ENCOUNTERED: return flags->encountered;
        case IE4_CHARA_FLAG_RECRUITED:   return flags->recruited;
        case IE4_CHARA_FLAG_NEW:         return flags->isNew;
        case IE4_CHARA_FLAG_UNK5:        return flags->Unk484;
        case IE4_CHARA_FLAG_UNK6:        return flags->Unk584;
        default:                         return 0;
    }
}

/**
 * @brief Locates the word holding a flag, or 0 when there is none
 * @details The model indexed arrays spill into sExtension past the range the
 * game can store; every other array is only bounds checked. Both halves are
 * aligned on a word, so the bit index is `index & 31` either way.
 */
static uint32_t *flagWord(CharaFlags *flags, uint32_t array, uint32_t index) {
    uint32_t *base = modelArray(flags, array);

    if (base != 0) {
        if (index < IE4_CHARA_FLAG_GAME_MAX) {
            return &base[index >> 5];
        }

        if (index < IE4_CHARA_FLAG_MAX) {
            uint32_t row = array - IE4_CHARA_FLAG_ENCOUNTERED;

            return &sExtension[row][(index - IE4_CHARA_FLAG_GAME_MAX) >> 5];
        }

        return 0;
    }

    switch (array) {
        case IE4_CHARA_FLAG_UNLOCK_A:
            return index < 2560 ? &flags->unlockA[index >> 5] : 0;
        case IE4_CHARA_FLAG_UNLOCK_B:
            return index < 512 ? &flags->unlockB[index >> 5] : 0;
        case IE4_CHARA_FLAG_UNK9:
            return index < 128 ? &flags->Unk684[index >> 5] : 0;
        case IE4_CHARA_FLAG_UNK11:
            return index < 32 ? &flags->Unk714[index >> 5] : 0;
        default:
            return 0;
    }
}

int ie4CharaFlagGet(const CharaFlags *flags, uint32_t array, uint32_t index) {
    const uint32_t *word;

    if (flags == 0) {
        return 0;
    }

    word = flagWord(const_cast<CharaFlags *>(flags), array, index);

    if (word == 0) {
        return 0;
    }

    return (*word >> (index & 31)) & 1;
}

void ie4CharaFlagSet(CharaFlags *flags, uint32_t array, uint32_t index, int value) {
    uint32_t *word;

    if (flags == 0) {
        return;
    }

    word = flagWord(flags, array, index);

    // Out of range writes are dropped: the original threw std::out_of_range here
    if (word == 0) {
        return;
    }

    if (value != 0) {
        *word |= 1u << (index & 31);
    } else {
        *word &= ~(1u << (index & 31));
    }
}

/** @brief Whether an array id holds flags, as opposed to byte sized values */
static bool isFlagArray(uint32_t array) {
    switch (array) {
        case IE4_CHARA_FLAG_UNLOCK_A:
        case IE4_CHARA_FLAG_UNLOCK_B:
        case IE4_CHARA_FLAG_ENCOUNTERED:
        case IE4_CHARA_FLAG_RECRUITED:
        case IE4_CHARA_FLAG_NEW:
        case IE4_CHARA_FLAG_UNK5:
        case IE4_CHARA_FLAG_UNK6:
        case IE4_CHARA_FLAG_UNK9:
        case IE4_CHARA_FLAG_UNK11:
            return true;
        default:
            return false;
    }
}

/** @brief Shared body of the four flag setters the game exposes */
static int setModelFlag(uint32_t array, uint32_t model, int value) {
    CharaFlags *flags = ie4CharaFlags();

    if (flags == 0) {
        return 0;
    }

    ie4CharaFlagSet(flags, array, model, value);

    // What the original returned, out of its own bit arithmetic
    return (int)(model >> 5);
}

int ie4CharaFlagIsEncountered(int unused, uint32_t model) {
    (void)unused;

    return ie4CharaFlagGet(ie4CharaFlags(), IE4_CHARA_FLAG_ENCOUNTERED, model);
}

int ie4CharaFlagIsRecruited(int unused, uint32_t model) {
    (void)unused;

    return ie4CharaFlagGet(ie4CharaFlags(), IE4_CHARA_FLAG_RECRUITED, model);
}

int ie4CharaFlagSetEncountered(int unused, uint32_t model, int value) {
    (void)unused;

    return setModelFlag(IE4_CHARA_FLAG_ENCOUNTERED, model, value);
}

int ie4CharaFlagSetNew(int unused, uint32_t model, int value) {
    (void)unused;

    return setModelFlag(IE4_CHARA_FLAG_NEW, model, value);
}

int ie4CharaFlagSetUnk5(int unused, uint32_t model, int value) {
    (void)unused;

    return setModelFlag(IE4_CHARA_FLAG_UNK5, model, value);
}

int ie4CharaFlagSetUnk6(int unused, uint32_t model, int value) {
    (void)unused;

    return setModelFlag(IE4_CHARA_FLAG_UNK6, model, value);
}

void *ie4CharaFlagQuery(void *out, CharaFlags *flags, uint32_t array, uint32_t index) {
    uint8_t *result = static_cast<uint8_t *>(out);

    if (result == 0) {
        return out;
    }

    if (isFlagArray(array)) {
        result[0] = QUERY_TYPE_FLAG;
        result[4] = (uint8_t)ie4CharaFlagGet(flags, array, index);

        return out;
    }

    // The byte sized arrays answer a value, and so does an id with no array at
    // all, which is how the original handled anything it did not know
    uint32_t value = 0;

    if (flags != 0) {
        if (array == IE4_CHARA_FLAG_BYTE10 && index < sizeof(flags->Unk694)) {
            value = flags->Unk694[index];
        } else if (array == IE4_CHARA_FLAG_BYTE12 && index < sizeof(flags->Unk718)) {
            value = flags->Unk718[index];
        }
    }

    result[0] = QUERY_TYPE_VALUE;
    *reinterpret_cast<uint32_t *>(result + 4) = value;

    return out;
}

void ie4CharaMarkRecruited(CharaFlags *flags, const void *baseRecord, void *counterObject) {
    uint32_t model;

    if (flags == 0 || baseRecord == 0) {
        return;
    }

    model = *reinterpret_cast<const uint16_t *>(
        static_cast<const uint8_t *>(baseRecord) + IE4_CHARA_BASE_MODEL_OFFSET);

    // A character met for the very first time carries the "new" marker
    if (!ie4CharaFlagGet(flags, IE4_CHARA_FLAG_ENCOUNTERED, model)) {
        ie4CharaFlagSet(flags, IE4_CHARA_FLAG_NEW, model, 1);
    }

    // The tally only counts him the first time he actually joins
    if (!ie4CharaFlagGet(flags, IE4_CHARA_FLAG_RECRUITED, model) && counterObject != 0) {
        uint16_t *counter = reinterpret_cast<uint16_t *>(
            static_cast<uint8_t *>(counterObject) + IE4_CHARA_RECRUIT_COUNT_OFFSET);

        if (*counter < IE4_CHARA_RECRUIT_COUNT_MAX) {
            (*counter)++;
        }
    }

    ie4CharaFlagSet(flags, IE4_CHARA_FLAG_RECRUITED, model, 1);
    ie4CharaFlagSet(flags, IE4_CHARA_FLAG_ENCOUNTERED, model, 1);
}

void ie4CharaFlagMarkNew(CharaFlags *flags, uint32_t model, int value) {
    ie4CharaFlagSet(flags, IE4_CHARA_FLAG_NEW, model, value);
}
