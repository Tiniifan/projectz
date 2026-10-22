#ifndef IE4ROSTER_H
#define IE4ROSTER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Number of players that can be recruited at once
 * @details Retail caps this at 112 (an even older layout capped it at 100). The
 * limit is not a single constant in the binary: it is the capacity of the player
 * pool, the length of the order list and the size of both save sections at once,
 * so raising it means moving all four together.
 * @note Chrono Stone, which runs the same engine, ships 352 here
 */
#define IE4_PLAYER_MAX 336

/** @brief Number of teams the order list keeps a fixed slot block for */
#define IE4_TEAM_COUNT 3

/** @brief Number of players in one of those teams */
#define IE4_TEAM_SIZE 16

/**
 * @brief Number of order list entries reserved for the team blocks
 * @details Team 0 is not stored here: it is simply the first IE4_TEAM_SIZE
 * entries of the recruit list, so only the other IE4_TEAM_COUNT blocks get their
 * own slots.
 */
#define IE4_TEAM_SLOT_COUNT (IE4_TEAM_COUNT * IE4_TEAM_SIZE)

/** @brief Total number of entries in an OrderList */
#define IE4_ORDER_MAX (IE4_PLAYER_MAX + IE4_TEAM_SLOT_COUNT)

/**
 * @brief Index the team slot blocks start at, minus one team
 * @details The game addresses a team slot as `IE4_ORDER_TEAM_BASE + 16 * team`,
 * which lands on IE4_PLAYER_MAX for team 1. Team 0 resolves to index 0, the head
 * of the recruit list.
 */
#define IE4_ORDER_TEAM_BASE (IE4_PLAYER_MAX - IE4_TEAM_SIZE)

/** @brief Size in bytes of one serialised player, as stored in the save file */
#define IE4_PLAYER_BLOCK_SIZE 204

/** @brief Size in bytes of one player record as held in RAM by the pool */
#define IE4_PLAYER_RECORD_SIZE 252

/** @brief Size in bytes of the payload of save section IE4_SECTION_PLAYERS */
#define IE4_PLAYER_SECTION_SIZE (IE4_PLAYER_MAX * IE4_PLAYER_BLOCK_SIZE)

/** @brief Size in bytes of the payload of save section IE4_SECTION_ORDER */
#define IE4_ORDER_SECTION_SIZE (IE4_ORDER_MAX * 4)

/**
 * @brief Marker every save section starts with
 */
#define IE4_SECTION_MAGIC 0xFFFE

/**
 * @brief Marker every save section ends with
 * @details Followed by two bytes the game never reads back, so they are written
 * as zero here.
 */
#define IE4_SECTION_END 0xFEFF

/** @brief Save section holding the serialised player records */
#define IE4_SECTION_PLAYERS 7

/** @brief Save section holding the order list */
#define IE4_SECTION_ORDER 10

/**
 * @brief Upper bound the game accepts for a section size
 * @details Must match the patched save buffer size, otherwise every section
 * reader rejects the enlarged player section outright.
 */
#define IE4_SAVE_BUFFER_SIZE 0x18000

/**
 * @brief Header stored at the start of every save section
 * @details Laid out little-endian. `Unk` is written from whatever the game had
 * on its stack and is never read back.
 */
typedef struct
{
    /** @brief Always IE4_SECTION_MAGIC */
    uint16_t magic;
    /** @brief Never read back by the game */
    uint16_t Unk;
    /** @brief Section type in the low byte, payload size in the upper 24 bits */
    uint32_t typeSize;
} SaveSection;

/**
 * @brief One of the two pools the player manager hands records out of
 * @details The manager holds IE4_PLAYER_POOL_COUNT of these back to back at
 * `manager + 4`, 40 bytes apart. Pool 0 holds the recruited players, pool 1 a
 * small scratch pool for guests. `capacity` and `stride` are filled in at boot
 * from the table at 0x005B20FC, which is where the 112 of retail lives.
 */
typedef struct
{
    /** @brief Pool vtable */
    void *vtable;
    /** @brief Base of the record array, `capacity * stride` bytes */
    uint8_t *records;
    /** @brief Backing storage of the allocation bitmap */
    void *bitmapData;
    /** @brief Number of records currently allocated */
    uint32_t used;
    /** @brief Monotonic counter feeding the high half of a player id */
    uint32_t serial;
    /** @brief Number of records the pool can hold */
    uint16_t capacity;
    /** @brief Distance in bytes between two records, IE4_PLAYER_RECORD_SIZE */
    uint16_t stride;
    /** @brief Allocation bitmap object, passed by address to the bitmap helpers */
    uint8_t bitmap[16];
} PlayerPool;

/**
 * @brief Number of pools a player manager owns
 */
#define IE4_PLAYER_POOL_COUNT 2

/**
 * @brief Distance in bytes between two PlayerPool entries in a manager
 */
#define IE4_PLAYER_POOL_STRIDE 40

/**
 * @brief Ordered list of player ids, embedded in the party object
 * @details Entries 0 .. IE4_PLAYER_MAX-1 are the recruited players in display
 * order; the IE4_TEAM_SLOT_COUNT entries after them mirror ids already present
 * in that first block, one block of IE4_TEAM_SIZE per extra team.
 * @note This struct is what forces most of the patches: it sits inline in the
 * party object at PARTY_ORDER_OFFSET, so growing `ids` moves every field behind
 * it. The array itself keeps its address, which is why the many inline
 * `party + 340 + 4 * i` accesses scattered through the binary still work.
 */
typedef struct
{
    /** @brief Order list vtable */
    void *vtable;
    /** @brief Player ids, zero for an empty slot */
    uint32_t ids[IE4_ORDER_MAX];
    /** @brief Number of recruited players held in the first block of `ids` */
    uint32_t count;
    /** @brief Points back into the owning party object, at party + 4 */
    uint32_t owner;
    /** @brief Unknown, cleared by the party constructor */
    uint32_t Unk1;
    /** @brief Unknown, cleared by the party constructor */
    uint32_t Unk2;
} OrderList;

/**
 * @brief Offset of the OrderList inside the party object
 */
#define PARTY_ORDER_OFFSET 336

/**
 * @brief Handle the game passes to ie4OrderListAppend
 * @details Not a real object: the game takes the address of this byte and uses
 * it as an identifier, so only its address matters.
 */
extern uint8_t IE4_ORDER_HANDLE;

/**
 * @brief Appends a player id to the order list of the current party
 * @param handle Address of IE4_ORDER_HANDLE
 * @param playerId Position id of the player to append
 * @param position Insertion index, or -1 to append at the end
 * @param unk Unknown, always 0 on the save loading path
 * @return int Non zero when the id was inserted
 * @note Game function at 0x00250684
 */
int ie4OrderListAppend(int handle, uint32_t playerId, int position, int unk);

/**
 * @brief Looks a player id up in one of the blocks of an order list
 * @param order Order list to search
 * @param team Block to search: 0 for the recruit list, 1 to IE4_TEAM_COUNT for a
 * team block
 * @param playerId Position id to look for
 * @return int Index into `ids`, or a negative value when the id is absent
 * @note Game function at 0x0039BC10
 */
int ie4OrderListFind(OrderList *order, int team, uint32_t playerId);

/**
 * @brief Rebuilds the derived state of one block of an order list
 * @param order Order list to refresh
 * @param team Block that changed, 0 for the recruit list
 * @return int Unused
 * @note Game function at 0x0039B3C8
 */
int ie4OrderListRefresh(OrderList *order, int team);

/**
 * @brief Moves an entry of an order list to another index
 * @param order Order list to reorder
 * @param team Block the entry lives in
 * @param from Index to move
 * @param to Index to move it to
 * @return int Unused
 * @note Game function at 0x0039BCAC
 */
int ie4OrderListMove(OrderList *order, int team, int from, uint32_t to);

/**
 * @brief Reads the player section of a save file into the player pool
 * @param manager Player manager owning the pools
 * @param header Section header already read from the file
 * @param data First byte of the section payload
 * @param bufferEnd One past the last byte of the save buffer
 * @param outSize Receives the payload size that was consumed
 * @param cursor Receives the address one past the payload
 * @return uint32_t 1 on success, 0 when the section is missing or malformed
 * @details Replaces the game function at 0x002FD010, which hardcoded 112 players
 * (110 for the older layout). The number of players is taken from the declared
 * section size instead, so a retail save still loads unchanged into the enlarged
 * pool.
 * @warning The pool capacity must have been raised beforehand, otherwise records
 * past its end are dropped
 */
uint32_t ie4PlayerSectionRead(int manager, const SaveSection *header, uint8_t *data,
                              uint32_t bufferEnd, uint32_t *outSize, uint8_t **cursor);

/**
 * @brief Reads the order list section of a save file
 * @param order Order list to fill
 * @param bufBase First byte of the save buffer
 * @param bufSize Size of the save buffer in bytes
 * @param cursor In out cursor, moved past the section on success
 * @param outSize Receives the total number of bytes the section occupies
 * @return uint32_t 1 on success, 0 when the section is missing or malformed
 * @details Replaces the game function at 0x0039B0A8. The recruit count is
 * derived from the declared section size, and the team blocks are relocated to
 * IE4_PLAYER_MAX whatever count the file was written with, so 100, 112 and
 * IE4_PLAYER_MAX entry saves all load.
 */
uint32_t ie4OrderSectionRead(OrderList *order, uint8_t *bufBase, uint32_t bufSize,
                             uint8_t **cursor, uint32_t *outSize);

/**
 * @brief Writes the order list section of a save file
 * @param order Order list to serialise
 * @param bufBase First byte of the save buffer
 * @param bufSize Size of the save buffer in bytes
 * @param cursor In out cursor, moved past the section on success
 * @param outSize Receives the total number of bytes the section occupies
 * @return uint32_t 1 on success, 0 when the section does not fit
 * @details Replaces the game function at 0x0039C7A0, which wrote a fixed 640
 * byte payload.
 */
uint32_t ie4OrderSectionWrite(OrderList *order, uint8_t *bufBase, uint32_t bufSize,
                              uint8_t **cursor, uint32_t *outSize);

/**
 * @brief Removes a player from the order list of a party
 * @param order Order list to remove from
 * @param index Index of the entry to remove, below IE4_PLAYER_MAX
 * @param keep Non zero to keep the player registered outside the list
 * @param benchOnly Non zero to only bench the player instead of removing it
 * @return int 1 when the list was updated, 0 otherwise
 * @details Replaces the game function at 0x0039C2AC. The original shifted the
 * tail of the list through a 112 entry scratch array on its own stack frame,
 * which would overflow as soon as the list holds more than 112 players; this
 * version shifts in place instead. Everything else follows the original.
 */
int ie4OrderRemoveEntry(OrderList *order, uint32_t index, int keep, int benchOnly);

/**
 * @brief Set to 1 to time save-reader phases; 0 keeps profiling disabled
 * @details Emulator debugging aid. Each phase is timed with svcGetSystemTick
 * and emitted as a bounded `<phase>:<8 hex digits>` string through the
 * debug-string SVC (0x3D). Phases are: 1 the whole player section, 2 record
 * decoding, 3 stat building, 4 equipment/move/avatar refresh,
 * 5 the whole order section, 6 appends, and 7 team remapping. Keep disabled by
 * default in production.
 */
#define IE4_ROSTER_PROFILE 0

/**
 * @brief Holder of the character database the list builder filters against
 * @details Same shape as PARTY_OFFSET: a pointer to a pointer, the object
 * itself living at offset 16.
 */
#define IE4_CHARA_DB_HOLDER 0x5B09D0

/**
 * @brief Size in bytes of one entry of a PlayerListView
 */
#define IE4_LIST_ENTRY_SIZE 12

/**
 * @brief Offset of the pointer table inside a PlayerListView
 */
#define IE4_LIST_TABLE_OFFSET 8

/**
 * @brief Offset of the party back pointer inside a PlayerListView
 * @details Every caller stores the party there before asking for a build, but
 * none of them ever reads it back and none of them refreshes it between two
 * builds, so ie4PlayerListBuild takes the party from PARTY_OFFSET2 instead and
 * lets the enlarged pointer table grow straight over this field.
 */
#define IE4_LIST_PARTY_OFFSET 456

/**
 * @brief Offset of the in-object entry storage inside a PlayerListView
 * @details Only used as a fallback when no scratch buffer could be allocated;
 * it holds IE4_LIST_LEGACY_MAX entries and nothing more.
 */
#define IE4_LIST_STORAGE_OFFSET 460

/**
 * @brief Number of entries the in-object storage of a PlayerListView holds
 */
#define IE4_LIST_LEGACY_MAX 112

/**
 * @brief Number of scratch buffers kept for PlayerListView entry storage
 * @details A menu holds at most two views at once and a submenu opened over it
 * adds two more, so anything above four is margin. Buffers are allocated on
 * first use and are only ever taken back from a view whose object has since
 * died, which ie4PlayerListBuild recognises by IE4_LIST_VIEW_VTABLE no longer
 * sitting at its `+0`.
 */
#define IE4_LIST_POOL_SLOTS 12

/**
 * @brief Vtable every PlayerListView carries at its `+0`
 * @details Written last by the constructor the menus inline, after the base
 * class one at 0x005A5EFC. Finding it at a `+0` is how the entry storage pool
 * tells a view that is still alive from one whose object has been freed.
 */
#define IE4_LIST_VIEW_VTABLE 0x5A2ECC

/**
 * @brief Holder of the character base records the sort reads names from
 * @details Same shape as PARTY_OFFSET, the object itself living at offset 16.
 */
#define IE4_CHARA_BASE_HOLDER 0x5B09C8

/**
 * @brief Pointer to the name database, stored plain rather than behind a holder
 */
#define IE4_NAME_DB 0x5B0A38

/**
 * @brief Highest sort key ie4PlayerListSort knows, anything above sorts nothing
 */
#define IE4_LIST_SORT_KEY_MAX 13

/**
 * @brief Reorders the entries of a player list view
 * @param view PlayerListView to sort in place
 * @param key What to sort on: 1 name, 2 nickname, 3 level, 4 to 11 the eight
 * stats, 12 appearances, 13 goals. Anything else leaves every key at zero
 * @return uint32_t Non zero once the view was walked, 0 when a database was
 * missing
 * @details Replaces the game function at 0x0038A6F8, used by the sort submenu
 * of the player list and by the statistics screen, which sorts on 13 then on 12
 * to rank scorers and appearances. The original collected the sort keys in a
 * 112 entry array on its own stack frame, so a list of more than 112 players
 * wrote over the frame's saved registers and return address; this version keeps
 * them in a static IE4_PLAYER_MAX array instead. It also skips the null table
 * entries a team view pushes, which the original dereferenced blindly.
 * @note Keys compare unsigned, and only keys 1 and 2 sort ascending; the order
 * of the game's own selection sort is kept so ties land where they used to.
 */
uint32_t ie4PlayerListSort(int view, int key);

/**
 * @brief Fills the player list view every menu browses
 * @param view PlayerListView to fill, laid out as described below
 * @param team 0 for the recruit list, 1 to IE4_TEAM_COUNT for a team block
 * @param filter Position mask: bit 0 GK, bit 1 FW, bit 2 MF, bit 3 DF, bit 4
 * caps the source list to IE4_TEAM_SIZE entries. 0, or all four position bits
 * set, means no filtering
 * @param excludeId Position id to leave out of the list
 * @return int 1 when the view was filled, 0 when a manager was missing
 * @details Replaces the game function at 0x0038A41C, which every player listing
 * menu calls (31 call sites). The view is embedded at a fixed offset inside each
 * menu object and is 1804 bytes long:
 * - `+0` unknown
 * - `+4` number of entries
 * - `+8` table of pointers to the entries, one word each
 * - `+456` back pointer to the party object
 * - `+460` entry storage, IE4_LIST_ENTRY_SIZE bytes each
 * The original laid the table out for IE4_LIST_LEGACY_MAX entries, so the 113th
 * pointer landed on the party back pointer at `+456` and the menus capped at 112
 * players. Since every consumer reaches an entry through the pointer table and
 * never through `+460` or `+456` directly, this version moves the entry storage
 * to a scratch buffer, takes the party from PARTY_OFFSET2, and lets the table
 * grow in place, up to (1804 - 8) / 4 = 449 entries, which covers
 * IE4_PLAYER_MAX with room to spare.
 * @note An entry is `{ uint32 playerId, uint32 group, uint32 sourceIndex }`,
 * `group` being 0 for the first five source slots, 1 up to the sixteenth and 2
 * beyond. A team view pushes a null pointer for a slot it cannot resolve, so the
 * sixteen slots stay aligned.
 * @warning A list of IE4_LIST_LEGACY_MAX entries or fewer stays in the storage
 * the view carries; a longer one needs a scratch buffer and is capped back to
 * IE4_LIST_LEGACY_MAX when none can be had
 */
int ie4PlayerListBuild(int view, int team, int filter, uint32_t excludeId);

#ifdef __cplusplus
}
#endif

#endif // IE4ROSTER_H
