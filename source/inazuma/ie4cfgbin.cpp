#include <stdint.h>
#include "../clib/string.h"
#include "../inazuma/ie4cfgbin.h"
#include "../utils/crc32.h"

// The game's own code reads these offsets, keep the layouts in sync with it
static_assert(sizeof(CfgBinHeader) == 0x10, "CfgBinHeader must be 16 bytes");
static_assert(sizeof(CfgBinKeyHeader) == 0x10, "CfgBinKeyHeader must be 16 bytes");
static_assert(sizeof(CfgBinReader) == 0x94, "CfgBinReader must be 0x94 bytes");
static_assert(__builtin_offsetof(CfgBinReader, currentEntry) == 0x38, "CfgBinReader layout mismatch");
static_assert(__builtin_offsetof(CfgBinReader, ignoreUnknownTags) == 0x50, "CfgBinReader layout mismatch");
static_assert(__builtin_offsetof(CfgBinReader, textState) == 0x54, "CfgBinReader layout mismatch");
static_assert(sizeof(CfgBinArgs) == 0x24, "CfgBinArgs must be 0x24 bytes");
static_assert(__builtin_offsetof(CfgBinArgs, userData) == 0x10, "CfgBinArgs layout mismatch");
static_assert(__builtin_offsetof(CfgBinArgs, entryIndex) == 0x22, "CfgBinArgs layout mismatch");

/** @brief Size of the fixed part of an entry: its CRC32 and its parameter count */
#define ENTRY_PREFIX_SIZE 5

static inline uint32_t alignUp(uint32_t value, uint32_t alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

/**
 * @brief Offset of the parameter values from the start of an entry
 * @details The type tags take one byte per four parameters and the values start
 * on the next 4 byte boundary.
 */
static inline uint32_t entryValuesOffset(uint32_t paramCount) {
    return alignUp(ENTRY_PREFIX_SIZE + (paramCount + 3) / 4, 4);
}

/** @brief Total size of an entry, which keeps the next one 4 byte aligned */
static inline uint32_t entrySize(uint32_t paramCount) {
    return entryValuesOffset(paramCount) + paramCount * 4;
}

static inline uint32_t entryCrc(const uint8_t *entry) {
    return *reinterpret_cast<const uint32_t *>(entry);
}

static inline uint32_t entryParamCount(const uint8_t *entry) {
    return entry[4];
}

static bool isFooter(const uint8_t *data) {
    return safeMemcmp(data, CFGBIN_FOOTER_MAGIC, CFGBIN_FOOTER_MAGIC_SIZE) == 0;
}

/**
 * @brief Sorts a tag table by CRC32 so cfgBinReaderStep can binary search it
 * @note The game uses a cocktail shaker sort here, any sort gives the same table
 */
__attribute__((optimize("no-tree-loop-distribute-patterns")))
static void sortTags(CfgBinTag *tags, uint32_t count) {
    uint32_t i;

    for (i = 1; i < count; i++) {
        CfgBinTag tag = tags[i];
        uint32_t j = i;

        while (j > 0 && tags[j - 1].crc > tag.crc) {
            tags[j] = tags[j - 1];
            j--;
        }

        tags[j] = tag;
    }
}

static const CfgBinTag *findTag(const CfgBinReader *reader, uint32_t crc) {
    uint32_t low = 0;
    uint32_t high = reader->tagCount;

    if (reader->tags == 0) {
        return 0;
    }

    while (low < high) {
        uint32_t middle = low + (high - low) / 2;
        uint32_t middleCrc = reader->tags[middle].crc;

        if (middleCrc == crc) {
            return &reader->tags[middle];
        }

        if (middleCrc < crc) {
            low = middle + 1;
        } else {
            high = middle;
        }
    }

    return 0;
}

static const char *findKeyName(const CfgBinReader *reader, uint32_t crc) {
    uint32_t i;

    if (reader->keyHeader == 0 || reader->keys == 0 || reader->keyStrings == 0) {
        return 0;
    }

    for (i = 0; i < reader->keyHeader->keyCount; i++) {
        if (reader->keys[i].crc == crc) {
            return reader->keyStrings + reader->keys[i].nameOffset;
        }
    }

    return 0;
}

/** @brief Whether the whole entry starting at `entry` lies inside the file */
static bool entryFits(const CfgBinReader *reader, const uint8_t *entry) {
    uint32_t offset = (uint32_t)(entry - reader->data);

    if (offset + ENTRY_PREFIX_SIZE > reader->size) {
        return false;
    }

    return offset + entrySize(entryParamCount(entry)) <= reader->size;
}

static void dispatchEntry(CfgBinReader *reader, const uint8_t *entry) {
    uint32_t crc = entryCrc(entry);
    const CfgBinTag *tag = findTag(reader, crc);
    CfgBinArgs args;
    CfgBinArgIter it;

    if (tag == 0 && (reader->ignoreUnknownTags || reader->defaultHandler == 0)) {
        return;
    }

    args.reader = reader;
    args.types = entry + ENTRY_PREFIX_SIZE;
    args.values = reinterpret_cast<const uint32_t *>(entry + entryValuesOffset(entryParamCount(entry)));
    args.paramCount = (int32_t)entryParamCount(entry);
    args.userData = reader->userData;
    args.isBinary = 1;
    args.name = findKeyName(reader, crc);
    args.crc = crc;
    args.Unk20 = 0;
    args.entryIndex = (uint16_t)reader->entryIndex;

    it.args = &args;
    it.index = 0;

    if (tag != 0) {
        tag->handler(&it);
    } else {
        reader->defaultHandler(crc, &it);
    }
}

int cfgBinReaderSetTagTable(CfgBinReader *reader, CfgBinTag *tags, uint16_t *count, uint8_t *sorted) {
    if (tags == 0) {
        return 0;
    }

    // The table is static and shared by every reader of a file type, so it is
    // counted and sorted once, the first time it is installed
    if (!*sorted) {
        if (*count == 0) {
            while (tags[*count].crc != 0) {
                (*count)++;
            }
        }

        sortTags(tags, *count);
        *sorted = 1;
    }

    reader->tagCount = *count;
    reader->tags = tags;

    return 1;
}

int cfgBinReaderLoad(CfgBinReader *reader, const void *data, uint32_t size, uint8_t unk) {
    const uint8_t *bytes = static_cast<const uint8_t *>(data);

    reader->data = bytes;
    reader->size = size;
    reader->Unk18 = unk;
    reader->header = 0;
    reader->entries = 0;
    reader->strings = 0;
    reader->stringsCopy = 0;
    reader->currentEntry = 0;
    reader->entryIndex = 0;
    reader->keyHeader = 0;
    reader->keys = 0;
    reader->keyStrings = 0;
    reader->footer = 0;
    reader->isBinary = 0;
    reader->hasKeyTable = 0;
    reader->hasFooter = 0;

    // Same subset of the text interpreter state the original clears
    reader->textState[0] = 0;
    reader->textState[1] = 0;
    reader->textState[2] = 0;
    reader->textState[3] = 0;
    reader->textState[4] = 0;
    reader->textState[5] = 0;
    reader->textState[12] = 0;
    reader->textState[13] = 0;
    reader->textState[14] = 0;
    reader->textState[15] = 0;

    if (bytes == 0 || size < sizeof(CfgBinHeader)) {
        return 1;
    }

    // There is no magic at the start of the file: it is recognised by its size
    // matching the end of one of the four possible layouts exactly
    const CfgBinHeader *header = reinterpret_cast<const CfgBinHeader *>(bytes);
    uint32_t bodySize = alignUp((uint32_t)header->stringTableOffset + (uint32_t)header->stringTableLength, 16);

    if (size == bodySize) {
        reader->isBinary = 1;
    } else if (size == bodySize + CFGBIN_FOOTER_SIZE) {
        if (isFooter(bytes + bodySize)) {
            reader->isBinary = 1;
            reader->hasFooter = 1;
            reader->footer = bytes + bodySize;
        }
    } else if (bodySize < size && size - bodySize > CFGBIN_FOOTER_SIZE) {
        const CfgBinKeyHeader *keyHeader = reinterpret_cast<const CfgBinKeyHeader *>(bytes + bodySize);
        uint32_t keySize = alignUp(keyHeader->keyStringOffset + keyHeader->keyStringLength, 16);
        uint32_t keyEnd = bodySize + keySize;

        if (keyHeader->keyTableSize == keySize) {
            if (size == keyEnd) {
                reader->isBinary = 1;
            } else if (size == keyEnd + CFGBIN_FOOTER_SIZE && isFooter(bytes + keyEnd)) {
                reader->isBinary = 1;
                reader->hasFooter = 1;
                reader->footer = bytes + keyEnd;
            }

            if (reader->isBinary) {
                reader->hasKeyTable = 1;
                reader->keyHeader = keyHeader;
                reader->keys = reinterpret_cast<const CfgBinKeyEntry *>(keyHeader + 1);
                reader->keyStrings = reinterpret_cast<const char *>(keyHeader) + keyHeader->keyStringOffset;
            }
        }
    }

    // The original also scans the first bytes of a text file here, then drops
    // the result: nothing to do for them until cfgBinReaderRun
    if (reader->isBinary) {
        reader->header = header;
        reader->entries = bytes + sizeof(CfgBinHeader);
        reader->strings = reinterpret_cast<const char *>(bytes) + header->stringTableOffset;
    }

    return 1;
}

char *cfgBinReaderCopyStrings(CfgBinReader *reader, char *dest, uint32_t destSize, uint32_t *outSize) {
    uint32_t length;

    if (!reader->isBinary || dest == 0 || reader->strings == 0 || reader->strings[0] == '\0') {
        return 0;
    }

    length = (uint32_t)reader->header->stringTableLength;
    *outSize = length;

    if (length > destSize) {
        return 0;
    }

    safeMemcpy3(dest, reader->strings, length);
    reader->stringsCopy = dest;

    return dest;
}

const uint8_t *cfgBinReaderStep(CfgBinReader *reader, int dispatch) {
    const uint8_t *entry;

    if (!reader->isBinary || reader->data == 0 || reader->size == 0 || reader->header == 0) {
        return 0;
    }

    if (reader->entryIndex >= (uint32_t)reader->header->entryCount) {
        return 0;
    }

    if (reader->currentEntry == 0) {
        entry = reader->entries;
    } else {
        entry = reader->currentEntry + entrySize(entryParamCount(reader->currentEntry));
    }

    // A header announcing more entries than the file holds would otherwise send
    // garbage to the handlers
    if (!entryFits(reader, entry)) {
        return 0;
    }

    reader->currentEntry = entry;

    if (dispatch) {
        dispatchEntry(reader, entry);
    }

    reader->entryIndex++;

    return entry;
}

const char *cfgBinReaderGetString(CfgBinReader *reader, int32_t offset) {
    const char *strings;

    if (!reader->isBinary || reader->data == 0 || reader->size == 0 || reader->header == 0 || reader->strings == 0) {
        return 0;
    }

    if (offset == CFGBIN_NULL_STRING) {
        return "";
    }

    if (offset < 0 || (uint32_t)offset >= (uint32_t)reader->header->stringTableLength) {
        return 0;
    }

    strings = reader->stringsCopy != 0 ? reader->stringsCopy : reader->strings;

    return strings + offset;
}

/**
 * @brief Fetches the parameter under the cursor
 * @return false when the entry has no parameter at that index, which the
 * original never checks: it would decode whatever follows the entry
 */
static bool fetchArg(const CfgBinArgIter *it, uint32_t *type, const uint32_t **value) {
    const CfgBinArgs *args = it->args;
    int32_t index = it->index;

    if (index < 0 || index >= args->paramCount) {
        return false;
    }

    *type = (args->types[index / 4] >> ((index % 4) * 2)) & 3;
    *value = &args->values[index];

    return true;
}

static int32_t argToInt(const CfgBinArgIter *it) {
    uint32_t type;
    const uint32_t *value;

    if (!fetchArg(it, &type, &value)) {
        return 0;
    }

    if (type == CFGBIN_TYPE_INT) {
        return (int32_t)*value;
    }

    if (type == CFGBIN_TYPE_FLOAT) {
        return (int32_t)*reinterpret_cast<const float *>(value);
    }

    return 0;
}

static float argToFloat(const CfgBinArgIter *it) {
    uint32_t type;
    const uint32_t *value;

    if (!fetchArg(it, &type, &value)) {
        return 0.0f;
    }

    if (type == CFGBIN_TYPE_FLOAT) {
        return *reinterpret_cast<const float *>(value);
    }

    if (type == CFGBIN_TYPE_INT) {
        return (float)(int32_t)*value;
    }

    return 0.0f;
}

static const char *argToString(const CfgBinArgIter *it) {
    uint32_t type;
    const uint32_t *value;

    if (!fetchArg(it, &type, &value)) {
        return 0;
    }

    if (type == CFGBIN_TYPE_STRING_PTR) {
        return reinterpret_cast<const char *>(*value);
    }

    if (type == CFGBIN_TYPE_STRING) {
        return cfgBinReaderGetString(it->args->reader, (int32_t)*value);
    }

    return 0;
}

int32_t cfgBinArgReadInt(CfgBinArgIter *it) {
    int32_t value = argToInt(it);

    it->index++;

    return value;
}

float cfgBinArgReadFloat(CfgBinArgIter *it) {
    float value = argToFloat(it);

    it->index++;

    return value;
}

uint32_t cfgBinArgReadHash(CfgBinArgIter *it) {
    uint32_t type;
    const uint32_t *value;
    uint32_t hash = 0;

    if (fetchArg(it, &type, &value)) {
        if (type == CFGBIN_TYPE_INT) {
            hash = *value;
        } else if (type == CFGBIN_TYPE_STRING || type == CFGBIN_TYPE_STRING_PTR) {
            // getCrc32 already returns 0 for a null or empty string
            const char *string = argToString(it);

            hash = (uint32_t)getCrc32(reinterpret_cast<uint8_t *>(const_cast<char *>(string)), 0, 0);
        }
    }

    it->index++;

    return hash;
}

const char *cfgBinArgReadString(CfgBinArgIter *it) {
    const char *value = argToString(it);

    it->index++;

    return value;
}

int32_t cfgBinArgPeekInt(CfgBinArgIter *it) {
    return argToInt(it);
}

float cfgBinArgPeekFloat(CfgBinArgIter *it) {
    return argToFloat(it);
}

const char *cfgBinArgPeekString(CfgBinArgIter *it) {
    return argToString(it);
}
