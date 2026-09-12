#ifndef IE4CFGBIN_H
#define IE4CFGBIN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Value type tags of a cfg.bin entry parameter, two bits per parameter
 * @details Stored four per byte right after the parameter count of an entry,
 * lowest bits first. Binary files only ever contain the first three: the fourth
 * is produced by the text form of the format, which stores a pointer to the
 * string itself instead of an offset into the string table.
 */
#define CFGBIN_TYPE_STRING     0
#define CFGBIN_TYPE_INT        1
#define CFGBIN_TYPE_FLOAT      2
#define CFGBIN_TYPE_STRING_PTR 3

/**
 * @brief Offset stored in place of a string table offset for a null string
 */
#define CFGBIN_NULL_STRING -1

/**
 * @brief Size in bytes of the optional footer closing a cfg.bin file
 * @details Starts with CFGBIN_FOOTER_MAGIC, then holds the encoding byte
 * (0 Shift-JIS, 1 UTF-8) and padding up to 16 bytes.
 */
#define CFGBIN_FOOTER_SIZE 16

/**
 * @brief First bytes of the optional footer, "\x01t2b\xFE"
 */
#define CFGBIN_FOOTER_MAGIC "\x01t2b\xFE"

/**
 * @brief Length of CFGBIN_FOOTER_MAGIC, the only part of the footer the game checks
 */
#define CFGBIN_FOOTER_MAGIC_SIZE 5

/**
 * @brief Header at offset 0 of every cfg.bin file
 * @details The file is laid out as:
 * `[ header ][ entries ][ strings ][ key table ][ footer ]`, where the string
 * table starts on a 16 byte boundary and both the key table and the footer are
 * optional. Entries start right after this header, at offset 0x10.
 */
typedef struct
{
    /** @brief Number of entries, _BEGIN and _END markers included */
    int32_t entryCount;
    /** @brief Offset of the string table from the start of the file */
    int32_t stringTableOffset;
    /** @brief Size in bytes of the string table */
    int32_t stringTableLength;
    /** @brief Number of distinct strings in the string table */
    int32_t stringTableCount;
} CfgBinHeader;

/**
 * @brief Header of the optional key table naming every entry type
 * @details Follows the string table, on the next 16 byte boundary. The game only
 * uses it to hand a readable name to the entry handlers, lookups are made on the
 * CRC32 alone.
 */
typedef struct
{
    /** @brief Size in bytes of the whole key table, this header included */
    uint32_t keyTableSize;
    /** @brief Number of CfgBinKeyEntry following this header */
    uint32_t keyCount;
    /** @brief Offset of the key names from the start of the key table */
    uint32_t keyStringOffset;
    /** @brief Size in bytes of the key names */
    uint32_t keyStringLength;
} CfgBinKeyHeader;

/**
 * @brief One entry of the key table
 */
typedef struct
{
    /** @brief CRC32 of the key name, as stored at the start of each entry */
    uint32_t crc;
    /** @brief Offset of the key name from the start of the key names */
    uint32_t nameOffset;
} CfgBinKeyEntry;

struct CfgBinArgIter;

/**
 * @brief Callback run for every entry whose CRC32 matches a CfgBinTag
 * @param args Iterator over the parameters of the entry, see cfgBinArgReadInt
 * @return int Ignored by the reader
 */
typedef int (*CfgBinTagHandler)(struct CfgBinArgIter *args);

/**
 * @brief Callback run for entries no CfgBinTag matches, when one is installed
 * @param crc CRC32 of the entry name
 * @param args Iterator over the parameters of the entry
 */
typedef void (*CfgBinDefaultHandler)(uint32_t crc, struct CfgBinArgIter *args);

/**
 * @brief Associates an entry name with the handler that consumes it
 * @details The game keeps one static array of these per file type, terminated by
 * a zero CRC, with the CRC32 fields filled in by a static initializer (see the
 * table at 0x005B1BF0 for chara_base.cfg.bin). The array is sorted in place by
 * CRC32 the first time it is installed, so the reader can binary search it.
 */
typedef struct
{
    /** @brief CRC32 of the entry name, for example of "CHARA_BASE_INFO" */
    uint32_t crc;
    /** @brief Function called with the parameters of every matching entry */
    CfgBinTagHandler handler;
} CfgBinTag;

/**
 * @brief State of the game's cfg.bin reader, the CScriptInterpreter3Ex class
 * @details The game builds one on the stack for every file it parses:
 * cfgBinReaderInit, cfgBinReaderSetTagTable, cfgBinReaderSetUserData,
 * cfgBinReaderLoad and finally cfgBinReaderRun, which calls cfgBinReaderStep
 * until every entry has been dispatched.
 * @note The same class also interprets the text form of the format. Only the
 * binary side is documented here, the text side keeps its state in textState.
 */
typedef struct
{
    /** @brief Number of entries in tags */
    uint32_t tagCount;                       // 0x00
    /** @brief Handlers of the entry types this file may contain, sorted by CRC32 */
    CfgBinTag *tags;                         // 0x04
    /** @brief Object the handlers fill, handed to them as CfgBinArgs::userData */
    void *userData;                          // 0x08
    /** @brief Called for entries without a tag, unless ignoreUnknownTags is set */
    CfgBinDefaultHandler defaultHandler;     // 0x0C
    /** @brief Raw file loaded by cfgBinReaderLoad */
    const uint8_t *data;                     // 0x10
    /** @brief Size in bytes of data */
    uint32_t size;                           // 0x14
    /** @brief Last argument of cfgBinReaderLoad, never read by the binary reader */
    uint8_t Unk18;                           // 0x18
    uint8_t pad19[3];                        // 0x19
    /** @brief File header, null when the file is not binary */
    const CfgBinHeader *header;              // 0x1C
    /** @brief First entry, right after the header */
    const uint8_t *entries;                  // 0x20
    /** @brief String table inside data */
    const char *strings;                     // 0x24
    /** @brief Copy of the string table made by cfgBinReaderCopyStrings, if any */
    const char *stringsCopy;                 // 0x28
    uint32_t Unk2C[3];                       // 0x2C
    /** @brief Entry dispatched by the last cfgBinReaderStep, null before the first */
    const uint8_t *currentEntry;             // 0x38
    /** @brief Index of the next entry cfgBinReaderStep will dispatch */
    uint32_t entryIndex;                     // 0x3C
    /** @brief Key table header, null when the file has no key table */
    const CfgBinKeyHeader *keyHeader;        // 0x40
    /** @brief First key table entry */
    const CfgBinKeyEntry *keys;              // 0x44
    /** @brief Key names, indexed by CfgBinKeyEntry::nameOffset */
    const char *keyStrings;                  // 0x48
    /** @brief Footer, null when the file has none */
    const uint8_t *footer;                   // 0x4C
    /** @brief Skips entries without a tag instead of calling defaultHandler */
    uint8_t ignoreUnknownTags;               // 0x50
    /** @brief Set by cfgBinReaderLoad when data is a binary cfg.bin */
    uint8_t isBinary;                        // 0x51
    /** @brief Set by cfgBinReaderLoad when the file has a key table */
    uint8_t hasKeyTable;                     // 0x52
    /** @brief Set by cfgBinReaderLoad when the file has a footer */
    uint8_t hasFooter;                       // 0x53
    /** @brief Text interpreter state, pointers to buffers on cfgBinReaderRun's stack */
    uint32_t textState[16];                  // 0x54
} CfgBinReader;

/**
 * @brief Entry being dispatched to a handler, built on the stack by cfgBinReaderStep
 */
typedef struct
{
    /** @brief Reader dispatching the entry */
    CfgBinReader *reader;                    // 0x00
    /** @brief Parameter type tags, four per byte */
    const uint8_t *types;                    // 0x04
    /** @brief Raw parameter values, one 32 bit word each */
    const uint32_t *values;                  // 0x08
    /** @brief Number of parameters */
    int32_t paramCount;                      // 0x0C
    /** @brief Copy of CfgBinReader::userData */
    void *userData;                          // 0x10
    /** @brief 1 when dispatched by cfgBinReaderStep, 0 by the text interpreter */
    uint8_t isBinary;                        // 0x14
    uint8_t pad15[3];                        // 0x15
    /** @brief Entry name, from the key table for binary files, null without one */
    const char *name;                        // 0x18
    /** @brief CRC32 of the entry name */
    uint32_t crc;                            // 0x1C
    /** @brief Always 0 for binary files, set by the text interpreter */
    uint16_t Unk20;                          // 0x20
    /** @brief Index of the entry in the file */
    uint16_t entryIndex;                     // 0x22
} CfgBinArgs;

/**
 * @brief Cursor over the parameters of an entry, what every handler receives
 * @details The cfgBinArgRead* functions return the parameter at index and move
 * to the next one, the cfgBinArgPeek* functions leave index untouched.
 */
typedef struct CfgBinArgIter
{
    /** @brief Entry being read */
    CfgBinArgs *args;
    /** @brief Index of the next parameter */
    int32_t index;
} CfgBinArgIter;

/**
 * @brief Constructs a reader, clearing every field
 * @param reader Reader to construct, 0x94 bytes
 * @return int Always 1
 */
int cfgBinReaderInit(CfgBinReader *reader);

/**
 * @brief Sets the object handed to the handlers as CfgBinArgs::userData
 * @param reader Reader to configure
 * @param userData Object the handlers fill
 * @return CfgBinReader* The reader itself
 */
CfgBinReader *cfgBinReaderSetUserData(CfgBinReader *reader, void *userData);

/**
 * @brief Dispatches every entry of the loaded file to its handler
 * @param reader Reader a file was loaded into
 * @return int 1 once every entry was dispatched, 0 if nothing is loaded
 * @details Rewinds the reader, then calls cfgBinReaderStep until it returns null.
 * Files in the text form of the format go through the game's text interpreter
 * instead, which keeps its buffers on the stack of this function.
 */
int cfgBinReaderRun(CfgBinReader *reader);

/**
 * @brief Installs the handlers of the entry types a file may contain
 * @param reader Reader to configure
 * @param tags Handler array terminated by a zero CRC, sorted in place
 * @param count Number of handlers in tags, counted and stored here when it is 0
 * @param sorted Set to 1 once tags is sorted, so that is only done once
 * @return int 1 on success, 0 if tags is null
 * @note The game keeps sorted, a padding byte and count right before tags
 * @note Replaces the game function originally located at 0x001CFB70
 */
int cfgBinReaderSetTagTable(CfgBinReader *reader, CfgBinTag *tags, uint16_t *count, uint8_t *sorted);

/**
 * @brief Attaches a file to the reader and detects its layout
 * @param reader Reader to load the file into
 * @param data File contents, kept referenced by the reader
 * @param size Size of the file in bytes
 * @param unk Stored in CfgBinReader::Unk18
 * @return int Always 1, isBinary tells whether data was recognised
 * @details data is binary when its size matches exactly one of the four layouts
 * `[ body ]`, `[ body ][ footer ]`, `[ body ][ key table ]` and
 * `[ body ][ key table ][ footer ]`, where the body is the header, the entries
 * and the string table padded to 16 bytes. Anything else is left to the text
 * interpreter.
 * @note Replaces the game function originally located at 0x001CFBA8
 * @warning data must stay alive until the reader is done with it, unless the
 * strings were copied with cfgBinReaderCopyStrings
 */
int cfgBinReaderLoad(CfgBinReader *reader, const void *data, uint32_t size, uint8_t unk);

/**
 * @brief Copies the string table out of the file, so strings outlive it
 * @param reader Reader a binary file was loaded into
 * @param dest Buffer receiving the string table
 * @param destSize Size of dest in bytes
 * @param outSize Receives the size of the string table in bytes
 * @return char* dest on success, from then on every string the reader hands
 * out points into it. 0 if the file has no strings or dest is too small
 * @note Replaces the game function originally located at 0x001CD8B0
 */
char *cfgBinReaderCopyStrings(CfgBinReader *reader, char *dest, uint32_t destSize, uint32_t *outSize);

/**
 * @brief Moves to the next entry and dispatches it to its handler
 * @param reader Reader a binary file was loaded into
 * @param dispatch 0 to only move to the next entry without calling anything
 * @return const uint8_t* The entry reached, or null once every entry was read
 * @details Looks the entry CRC32 up in the tag table and calls the matching
 * handler, or defaultHandler when there is none and ignoreUnknownTags is clear.
 * Entries matching nothing are skipped.
 * @note Replaces the game function originally located at 0x001CDAC4. Unlike the
 * original, it stops instead of reading past the end of a truncated file
 */
const uint8_t *cfgBinReaderStep(CfgBinReader *reader, int dispatch);

/**
 * @brief Resolves a string table offset
 * @param reader Reader a binary file was loaded into
 * @param offset Offset read from a CFGBIN_TYPE_STRING parameter
 * @return const char* The string, "" for CFGBIN_NULL_STRING, 0 for an offset
 * outside the string table
 * @note Replaces the game function originally located at 0x0048B5A4
 */
const char *cfgBinReaderGetString(CfgBinReader *reader, int32_t offset);

/**
 * @brief Reads the next parameter as an integer
 * @param it Cursor over the parameters of the entry
 * @return int32_t The value, truncated when it is a float, 0 when it is a string
 * or when the entry has no parameter left
 * @note Replaces the game function originally located at 0x0012D07C
 */
int32_t cfgBinArgReadInt(CfgBinArgIter *it);

/**
 * @brief Reads the next parameter as a float
 * @param it Cursor over the parameters of the entry
 * @return float The value, converted when it is an integer, 0 when it is a
 * string or when the entry has no parameter left
 * @note Replaces the game function originally located at 0x0012EDD0
 */
float cfgBinArgReadFloat(CfgBinArgIter *it);

/**
 * @brief Reads the next parameter as a hash
 * @param it Cursor over the parameters of the entry
 * @return uint32_t The CRC32 of the value when it is a string, the value itself
 * when it is an integer, 0 otherwise
 * @details This is how cfg.bin files reference each other: a field may hold
 * either a name or the CRC32 of that name.
 * @note Replaces the game function originally located at 0x0012CFF0
 */
uint32_t cfgBinArgReadHash(CfgBinArgIter *it);

/**
 * @brief Reads the next parameter as a string
 * @param it Cursor over the parameters of the entry
 * @return const char* The string, 0 when the value is not a string or when the
 * entry has no parameter left
 * @note Replaces the game function originally located at 0x0012F8A0
 */
const char *cfgBinArgReadString(CfgBinArgIter *it);

/**
 * @brief Same as cfgBinArgReadInt, without moving to the next parameter
 * @note Replaces the game function originally located at 0x00128C70
 */
int32_t cfgBinArgPeekInt(CfgBinArgIter *it);

/**
 * @brief Same as cfgBinArgReadFloat, without moving to the next parameter
 * @note Replaces the game function originally located at 0x0012B610
 */
float cfgBinArgPeekFloat(CfgBinArgIter *it);

/**
 * @brief Same as cfgBinArgReadString, without moving to the next parameter
 * @note Replaces the game function originally located at 0x0012C298
 */
const char *cfgBinArgPeekString(CfgBinArgIter *it);

#ifdef __cplusplus
}
#endif

#endif // IE4CFGBIN_H
