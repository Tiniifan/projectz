#include <stdint.h>
#include "../clib/string.h"
#include "../inazuma/ie4party.h"
#include "../inazuma/ie4roster.h"

#if IE4_ROSTER_PROFILE

/**
 * @brief Reads the low word of the system tick counter
 * @return uint32_t Tick, counting at 268111856 Hz
 */
static inline uint32_t profTick(void) {
    register uint32_t low __asm__("r0");
    register uint32_t high __asm__("r1");

    __asm__ volatile("svc 0x28" : "=r"(low), "=r"(high) :: "memory");
    (void)high;
    return low;
}

/**
 * @brief Reports one timing by touching an address that encodes it
 * @param tag Phase, 1 to 7
 * @param ticks Ticks the phase took
 */
static void profReport(uint32_t tag, uint32_t ticks) {
    volatile uint32_t *probe;

    probe = (volatile uint32_t *)(((tag & 7) << 24) | ((ticks >> 8) & 0xFFFFFF));
    (void)*probe;
}

#define PROF_DECL(name) uint32_t name = 0
#define PROF_MARK(name) name = profTick()
#define PROF_ADD(total, mark) total += profTick() - (mark)
#define PROF_SEND(tag, total) profReport((tag), (total))

#else

#define PROF_DECL(name) ((void)0)
#define PROF_MARK(name) ((void)0)
#define PROF_ADD(total, mark) ((void)0)
#define PROF_SEND(tag, total) ((void)0)

#endif

/**
 * @brief Resets an allocation bitmap to all free
 * @param bitmap Bitmap object, taken by address from a PlayerPool
 * @note Game function at 0x0053C840
 */
extern "C" int bitmapClear(void *bitmap);

/**
 * @brief Finds the first bit holding `value` at or after `from`
 * @param bitmap Bitmap object, taken by address from a PlayerPool
 * @param value Bit value to look for, 0 for a free slot and 1 for a used one
 * @param from Index to start scanning at
 * @return int Index of the matching bit, or a negative value when there is none
 * @note Game function at 0x00498D00
 */
extern "C" int bitmapFind(void *bitmap, int value, int from);

/**
 * @brief Writes one bit of an allocation bitmap
 * @param bitmap Bitmap object, taken by address from a PlayerPool
 * @param index Bit to write
 * @param value Value to write
 * @note Game function at 0x0044E40C
 */
extern "C" int bitmapSet(void *bitmap, int index, int value);

/**
 * @brief Prepares a freshly taken pool record before it is filled in
 * @param record Record to prepare
 * @return void* The same record, or NULL when it could not be prepared
 * @note Game function at 0x0024F37C
 */
extern "C" void *ie4PlayerRecordInit(void *record);

/**
 * @brief Expands one serialised player block into a live player record
 * @param record Destination, the record payload at `record + 4`
 * @param block Source, IE4_PLAYER_BLOCK_SIZE bytes read from the save file
 * @return int The destination
 * @note Game function at 0x0033AD30
 */
extern "C" int deserializePlayerData(int record, const int *block);

/**
 * @brief Recomputes the ten derived stats cached at the end of a player record
 * @param stats Receives the ten values
 * @param kind Player kind, taken from record byte 22
 * @param playerId Position id of the player
 * @note Game function at 0x0033E404
 */
extern "C" void ie4PlayerBuildStats(int16_t *stats, uint8_t kind, uint32_t playerId);

/**
 * @brief Rebuilds the equipment state of a player that was just loaded
 * @param manager Player manager owning the record
 * @param playerId Position id of the player
 * @param unk Unknown, always 1 on the save loading path
 * @note Game function at 0x002FC444
 */
extern "C" int ie4PlayerRefreshEquipment(int manager, uint32_t playerId, int unk);

/**
 * @brief Rebuilds the move state of a player that was just loaded
 * @param manager Player manager owning the record
 * @param playerId Position id of the player
 * @param unk Unknown, always 1 on the save loading path
 * @note Game function at 0x002FC764
 */
extern "C" int ie4PlayerRefreshMoves(int manager, uint32_t playerId, int unk);

/**
 * @brief Registers the avatar a loaded player carries
 * @param slot Avatar slot, taken from record byte 56
 * @param playerId Position id of the player
 * @note Game function at 0x003407CC
 */
extern "C" int ie4PlayerRegisterAvatar(uint8_t slot, uint32_t playerId);

/**
 * @brief Clears one of the five per player caches when a player leaves the team
 * @param manager Player manager owning the record
 * @param playerId Position id of the player being removed
 * @param index Cache index, 0 to 4
 * @note Game function at 0x002FC25C
 */
extern "C" int ie4PartyClearCache(int manager, uint32_t playerId, int index);

/**
 * @brief Drops the record of a player that left the team for good
 * @param manager Player manager owning the record
 * @param playerId Position id of the player being removed
 * @note Game function at 0x002FBA50
 */
extern "C" int ie4PartyDropPlayer(int manager, uint32_t playerId);

/**
 * @brief Looks a player up in the character database
 * @param handle Address of IE4_CHARA_HANDLE
 * @param playerId Position id to look for
 * @return int Database entry, or 0 when the player is unknown
 * @note Game function at 0x0024F5F8
 */
extern "C" int ie4CharaFind(int handle, uint32_t playerId);

/**
 * @brief Looks a player up in the secondary character table
 * @param handle Address of IE4_CHARA_HANDLE
 * @param playerId Position id to look for
 * @return int Table entry, or 0 when the player is unknown
 * @note Game function at 0x0024F8CC
 */
extern "C" int ie4CharaFindAlt(int handle, uint32_t playerId);

/**
 * @brief Reads one of the game's global bit flags
 * @param handle Address of IE4_FLAG_HANDLE
 * @param flag Flag number
 * @return int Non zero when the flag is set
 * @note Game function at 0x001F2A8C
 */
extern "C" int getGlobalBitFlag(int handle, unsigned int flag);

/**
 * @brief Aborts with the game's assertion handler
 * @param a First value shown by the handler
 * @param b Second value shown by the handler
 * @note Game function at 0x00519028
 */
extern "C" int _assert_func(int a, int b);

/** @brief Handle identifying the character database, address only */
extern "C" uint8_t IE4_CHARA_HANDLE;

/** @brief Handle identifying the global bit flags, address only */
extern "C" uint8_t IE4_FLAG_HANDLE;

/**
 * @brief Reads a save section header without assuming the buffer is aligned
 * @param header Receives the decoded header
 * @param source First byte of the section
 */
static void readSectionHeader(SaveSection *header, const uint8_t *source) {
    header->magic = (uint16_t)(source[0] | (source[1] << 8));
    header->Unk = (uint16_t)(source[2] | (source[3] << 8));
    header->typeSize = (uint32_t)source[4] | ((uint32_t)source[5] << 8)
                     | ((uint32_t)source[6] << 16) | ((uint32_t)source[7] << 24);
}

/**
 * @brief Writes a save section header
 * @param destination First byte of the section
 * @param type Section type
 * @param size Payload size in bytes
 */
static void writeSectionHeader(uint8_t *destination, uint8_t type, uint32_t size) {
    uint32_t typeSize = ((uint32_t)type) | (size << 8);

    destination[0] = (uint8_t)(IE4_SECTION_MAGIC & 0xFF);
    destination[1] = (uint8_t)(IE4_SECTION_MAGIC >> 8);
    destination[2] = 0;
    destination[3] = 0;
    destination[4] = (uint8_t)(typeSize & 0xFF);
    destination[5] = (uint8_t)((typeSize >> 8) & 0xFF);
    destination[6] = (uint8_t)((typeSize >> 16) & 0xFF);
    destination[7] = (uint8_t)((typeSize >> 24) & 0xFF);
}

/**
 * @brief Reads the four byte marker closing a save section
 * @param source First byte of the marker
 * @return uint16_t The marker, IE4_SECTION_END for a well formed section
 */
static uint16_t readSectionEnd(const uint8_t *source) {
    return (uint16_t)(source[0] | (source[1] << 8));
}

/**
 * @brief Writes the four byte marker closing a save section
 * @param destination First byte of the marker
 */
static void writeSectionEnd(uint8_t *destination) {
    destination[0] = (uint8_t)(IE4_SECTION_END & 0xFF);
    destination[1] = (uint8_t)(IE4_SECTION_END >> 8);
    destination[2] = 0;
    destination[3] = 0;
}

/**
 * @brief Looks a character up in the database the list builder filters against
 * @param database Database object, taken from IE4_CHARA_DB_HOLDER
 * @param charaId Character id, read from player record offset 8
 * @param unk Unknown, always 0 on the list building path
 * @return int Database entry, or 0 when the character is unknown
 * @note Game function at 0x00493AE4
 */
extern "C" int ie4CharaLookup(int database, int charaId, int unk);

/**
 * @brief Finds the base record of a character, holding its name ids
 * @param database Object at offset 16 of IE4_CHARA_BASE_HOLDER
 * @param baseId Base id, read at offset 4 of a character record
 * @param unk Always 0 at every call site
 * @return int Base record, or 0 when there is none
 * @note Game function at 0x00493494
 */
extern "C" int ie4CharaBaseLookup(int database, int baseId, int unk);

/**
 * @brief Resolves one of the name ids of a character base record
 * @param database Pointer read straight out of IE4_NAME_DB
 * @param nameId Name id, at offset 8, or nickname id, at offset 12, of a base
 * @param unk Always 0 at every call site
 * @param unk2 Always 1 at every call site
 * @return int Name record, whose offset 38 is the sort rank, or 0
 * @note Game function at 0x00490978
 */
extern "C" int ie4NameLookup(int database, int nameId, int unk, int unk2);

/**
 * @brief Reads one of the eight stats of a player record
 * @param record Player record, as returned by getPlayerOffset
 * @param stat Stat index, 2 to 9
 * @return int Stat value
 * @note Game function at 0x00490484
 */
extern "C" int ie4PlayerStatValue(int record, int stat);

/**
 * @brief Allocates a block with the game's general purpose allocator
 * @param size Size in bytes
 * @return void* The block, or NULL when the allocation failed
 * @note Game function at 0x0052AB58
 */
extern "C" void *safeMalloc2(unsigned int size);

/**
 * @brief Reads a little-endian 32 bit value from a possibly unaligned address
 * @param source First byte of the value
 * @return uint32_t The value
 */
static uint32_t readU32(const uint8_t *source) {
    return (uint32_t)source[0] | ((uint32_t)source[1] << 8)
         | ((uint32_t)source[2] << 16) | ((uint32_t)source[3] << 24);
}

/**
 * @brief Writes a little-endian 32 bit value to a possibly unaligned address
 * @param destination First byte of the value
 * @param value Value to write
 */
static void writeU32(uint8_t *destination, uint32_t value) {
    destination[0] = (uint8_t)(value & 0xFF);
    destination[1] = (uint8_t)((value >> 8) & 0xFF);
    destination[2] = (uint8_t)((value >> 16) & 0xFF);
    destination[3] = (uint8_t)(value >> 24);
}

__attribute__((optimize("no-tree-loop-distribute-patterns")))
uint32_t ie4PlayerSectionRead(int manager, const SaveSection *header, uint8_t *data,
                              uint32_t bufferEnd, uint32_t *outSize, uint8_t **cursor) {
    PlayerPool *pool = (PlayerPool *)(manager + 4);
    uint32_t size = header->typeSize >> 8;
    uint8_t *end = data + size;
    uint32_t count;
    uint32_t i;
    PROF_DECL(profAll);
    PROF_DECL(profStart);
    PROF_DECL(profDecode);
    PROF_DECL(profStats);
    PROF_DECL(profRefresh);
    PROF_DECL(profMark);

    PROF_MARK(profStart);

    // The game publishes the cursor before validating anything, keep doing so
    *cursor = end;

    if ((uint32_t)end > bufferEnd) {
        return 0;
    }
    if (header->magic != IE4_SECTION_MAGIC) {
        return 0;
    }
    if ((header->typeSize & 0xFF) != IE4_SECTION_PLAYERS) {
        return 0;
    }
    if (size > IE4_SAVE_BUFFER_SIZE) {
        return 0;
    }

    // Retail writes 112 blocks and the oldest layout 110, so the count comes
    // from the section itself rather than from a constant
    count = size / IE4_PLAYER_BLOCK_SIZE;
    if (count > pool->capacity) {
        count = pool->capacity;
    }

    if (count != 0) {
        bitmapClear(pool->bitmap);
        pool->used = 0;

        for (i = 0; i < count; i++) {
            const uint8_t *block = data + i * IE4_PLAYER_BLOCK_SIZE;
            uint8_t *record;
            uint32_t playerId;
            int16_t stats[10];
            int wanted;
            int slot;
            int k;

            // A block whose player hash is zero is an empty save slot
            if (readU32(block + 4) == 0) {
                continue;
            }

            wanted = (int)(readU32(block) & 0xFFF);
            if (wanted >= (int)pool->capacity) {
                continue;
            }

            slot = bitmapFind(pool->bitmap, 0, wanted);
            if (slot < 0 || slot >= (int)pool->capacity) {
                continue;
            }

            record = pool->records + slot * pool->stride;
            PROF_MARK(profMark);
            record = (uint8_t *)ie4PlayerRecordInit(record);
            if (record == 0) {
                continue;
            }

            deserializePlayerData((int)(record + 4), (const int *)block);
            PROF_ADD(profDecode, profMark);
            playerId = *(uint32_t *)(record + 4);

            PROF_MARK(profMark);
            ie4PlayerBuildStats(stats, record[22], playerId);
            for (k = 0; k < 10; k++) {
                *(uint16_t *)(record + 208 + 2 * k) = (uint16_t)stats[k];
            }
            PROF_ADD(profStats, profMark);

            PROF_MARK(profMark);
            ie4PlayerRefreshEquipment(manager, playerId, 1);
            ie4PlayerRefreshMoves(manager, playerId, 1);

            if (*(uint32_t *)(record + 52) != 0) {
                ie4PlayerRegisterAvatar(record[56], playerId);
            }
            PROF_ADD(profRefresh, profMark);

            bitmapSet(pool->bitmap, slot, 1);
            pool->serial++;
            pool->used++;
        }
    }

    PROF_ADD(profAll, profStart);
    PROF_SEND(1, profAll);
    PROF_SEND(2, profDecode);
    PROF_SEND(3, profStats);
    PROF_SEND(4, profRefresh);

    *outSize = size;
    return 1;
}

__attribute__((optimize("no-tree-loop-distribute-patterns")))
uint32_t ie4OrderSectionRead(OrderList *order, uint8_t *bufBase, uint32_t bufSize,
                             uint8_t **cursor, uint32_t *outSize) {
    SaveSection header;
    uint8_t *head;
    uint8_t *data;
    uint8_t *end;
    uint8_t *trailer;
    uint32_t size;
    uint32_t entries;
    uint32_t recruited;
    uint32_t i;
    PROF_DECL(profAll);
    PROF_DECL(profStart);
    PROF_DECL(profAppend);
    PROF_DECL(profRemap);
    PROF_DECL(profMark);

    PROF_MARK(profStart);

    if (outSize != 0) {
        *outSize = 0;
    }
    if (bufBase == 0 || bufSize == 0 || cursor == 0) {
        return 0;
    }

    head = *cursor;
    if (head == 0 || head < bufBase) {
        return 0;
    }

    end = bufBase + bufSize;
    data = head + 8;
    if (data > end) {
        return 0;
    }

    readSectionHeader(&header, head);
    size = header.typeSize >> 8;

    // Cleared before the header is validated, exactly like the original
    for (i = 0; i < IE4_ORDER_MAX; i++) {
        order->ids[i] = 0;
    }
    order->count = 0;

    if (data + size > end) {
        return 0;
    }
    if (header.magic != IE4_SECTION_MAGIC) {
        return 0;
    }
    if ((header.typeSize & 0xFF) != IE4_SECTION_ORDER) {
        return 0;
    }
    if (size > IE4_SAVE_BUFFER_SIZE) {
        return 0;
    }

    // The team blocks always close the section, so whatever comes before them is
    // the recruit list, be it 100, 112 or IE4_PLAYER_MAX entries long
    entries = size / 4;
    if (entries < IE4_TEAM_SLOT_COUNT) {
        return 0;
    }
    recruited = entries - IE4_TEAM_SLOT_COUNT;
    if (recruited > IE4_PLAYER_MAX) {
        recruited = IE4_PLAYER_MAX;
    }

    PROF_MARK(profMark);
    for (i = 0; i < recruited; i++) {
        ie4OrderListAppend((int)&IE4_ORDER_HANDLE, readU32(data + 4 * i), -1, 0);
    }
    PROF_ADD(profAppend, profMark);

    // Team slots hold ids that are already in the recruit list, so they are
    // rewritten from it and land on their new index whatever the file said
    PROF_MARK(profMark);
    for (i = 0; i < IE4_TEAM_SLOT_COUNT; i++) {
        int found = ie4OrderListFind(order, 0, readU32(data + 4 * (recruited + i)));

        if (found >= 0) {
            order->ids[IE4_PLAYER_MAX + i] = order->ids[found];
        }
    }
    PROF_ADD(profRemap, profMark);

    trailer = data + size;
    if (trailer + 4 > end) {
        return 0;
    }
    if (readSectionEnd(trailer) != IE4_SECTION_END) {
        return 0;
    }

    PROF_ADD(profAll, profStart);
    PROF_SEND(5, profAll);
    PROF_SEND(6, profAppend);
    PROF_SEND(7, profRemap);

    *cursor = trailer + 4;
    if (outSize != 0) {
        *outSize = size + 12;
    }
    return 1;
}

__attribute__((optimize("no-tree-loop-distribute-patterns")))
uint32_t ie4OrderSectionWrite(OrderList *order, uint8_t *bufBase, uint32_t bufSize,
                              uint8_t **cursor, uint32_t *outSize) {
    uint8_t *head;
    uint8_t *data;
    uint8_t *end;
    uint8_t *trailer;
    uint32_t i;

    if (outSize != 0) {
        *outSize = 0;
    }
    if (bufBase == 0 || bufSize == 0 || cursor == 0) {
        return 0;
    }

    head = *cursor;
    if (head == 0 || head < bufBase) {
        return 0;
    }

    end = bufBase + bufSize;
    data = head + 8;
    if (data >= end) {
        return 0;
    }
    if (data + IE4_ORDER_SECTION_SIZE >= end) {
        return 0;
    }

    safeMemset(data, IE4_ORDER_SECTION_SIZE);
    for (i = 0; i < IE4_ORDER_MAX; i++) {
        writeU32(data + 4 * i, order->ids[i]);
    }

    writeSectionHeader(head, IE4_SECTION_ORDER, IE4_ORDER_SECTION_SIZE);

    trailer = data + IE4_ORDER_SECTION_SIZE;
    if (trailer + 4 >= end) {
        return 0;
    }
    writeSectionEnd(trailer);

    *cursor = trailer + 4;
    if (outSize != 0) {
        *outSize = IE4_ORDER_SECTION_SIZE + 12;
    }
    return 1;
}

__attribute__((optimize("no-tree-loop-distribute-patterns")))
__attribute__((optimize("no-tree-loop-distribute-patterns")))
int ie4OrderRemoveEntry(OrderList *order, uint32_t index, int keep, int benchOnly) {
    void *holder = *(void **)PARTY_OFFSET;
    uint32_t removedId;
    int manager;
    int i;

    if (index >= IE4_PLAYER_MAX) {
        return 0;
    }

    if (benchOnly != 0) {
        int target;
        int scan;
        int limit;

        if (holder == 0) {
            _assert_func((int)holder, (int)index);
        }

        manager = *((int *)holder + 4);
        if (manager == 0) {
            return 0;
        }

        target = getPlayerOffset(manager, (int)order->ids[index]);
        if (target == 0) {
            return 0;
        }

        limit = (int)order->count;
        if (limit >= IE4_TEAM_SIZE) {
            limit = IE4_TEAM_SIZE;
        }

        for (scan = limit - 1; scan >= 0; scan--) {
            int other = getPlayerOffset(manager, (int)order->ids[scan]);

            if (other != 0 && *(uint8_t *)(other + 27) == 0) {
                *(uint8_t *)(target + 27) = 1;
                ie4OrderListMove(order, 0, (int)index, (uint32_t)scan);
                break;
            }
        }

        return 1;
    }

    removedId = order->ids[index];

    if (index >= IE4_PLAYER_MAX - 1) {
        order->ids[index] = 0;
    } else {
        uint8_t *owner = (uint8_t *)order->owner;
        uint32_t moved = 0;

        if (order->count > index + 1) {
            moved = order->count - index - 1;
        }

        // The original shifted through a 112 entry scratch array on its stack;
        // the destination is below the source so a forward copy is enough and
        // stays correct however long the list gets
        for (i = 0; (uint32_t)i < moved; i++) {
            order->ids[index + i] = order->ids[index + i + 1];
        }
        if (order->count != 0) {
            order->ids[order->count - 1] = 0;
        }

        if (index < IE4_TEAM_SIZE - 1) {
            uint8_t *slots = owner + 36;
            uint32_t last = order->count;
            uint8_t saved;

            if (last > IE4_TEAM_SIZE) {
                last = IE4_TEAM_SIZE;
            }
            if (last > index) {
                saved = slots[index];
                for (i = 0; (uint32_t)i < last - index - 1; i++) {
                    slots[index + i] = slots[index + i + 1];
                }
                slots[last - 1] = saved;
            }
        }

        if (index < 4) {
            uint8_t *slots = owner + 12;
            uint32_t last = order->count;
            uint8_t saved;

            if (last > 5) {
                last = 5;
            }
            if (last > index) {
                saved = slots[index];
                for (i = 0; (uint32_t)i < last - index - 1; i++) {
                    slots[index + i] = slots[index + i + 1];
                }
                slots[last - 1] = saved;
            }
        }

        ie4OrderListRefresh(order, 0);
    }

    for (i = 1; i <= IE4_TEAM_COUNT; i++) {
        int found = ie4OrderListFind(order, i, removedId);

        if (found >= 0) {
            order->ids[found] = 0;
            ie4OrderListRefresh(order, i);
        }
    }

    if (holder == 0) {
        _assert_func((int)holder, (int)index);
        return 0;
    }

    manager = *((int *)holder + 4);
    if (manager != 0) {
        for (i = 0; i < 5; i++) {
            ie4PartyClearCache(manager, removedId, i);
        }
        if (keep == 0) {
            ie4PartyDropPlayer(manager, removedId);
        }
    }

    order->count--;

    if (index < IE4_PLAYER_MAX - 1) {
        int scan;

        scan = (int)(order->count - 2);
        if ((uint32_t)(order->count - 2) > 5) {
            scan = 5;
        }
        for (; scan >= 0; scan--) {
            int entry = ie4CharaFind((int)&IE4_CHARA_HANDLE, order->ids[scan]);

            if (entry != 0 && *(uint8_t *)(entry + 27) != 0) {
                uint32_t to = order->count;

                if (to > (uint32_t)(scan + 1)) {
                    to = (uint32_t)(scan + 1);
                }
                ie4OrderListMove(order, 0, scan, to);
            }
        }

        scan = (int)(order->count - 2);
        if ((uint32_t)(order->count - 2) > 0xC) {
            scan = 12;
        }
        for (; scan >= 5; scan--) {
            int entry = ie4CharaFindAlt((int)&IE4_CHARA_HANDLE, order->ids[scan]);

            if (entry == 0) {
                continue;
            }

            if (getGlobalBitFlag((int)&IE4_FLAG_HANDLE, 0x7DA) && *(uint16_t *)(entry + 4) == 9) {
                uint32_t to = order->count;

                if (to > (uint32_t)(scan + 1)) {
                    to = (uint32_t)(scan + 1);
                }
                ie4OrderListMove(order, 0, scan, to);
            }
            if (getGlobalBitFlag((int)&IE4_FLAG_HANDLE, 0x7DB) && *(uint16_t *)(entry + 4) == 10) {
                uint32_t to = order->count;

                if (to > (uint32_t)(scan + 1)) {
                    to = (uint32_t)(scan + 1);
                }
                ie4OrderListMove(order, 0, scan, to);
            }
        }
    }

    return 1;
}

/** @brief View a pooled scratch buffer currently belongs to, 0 when free */
static int g_listView[IE4_LIST_POOL_SLOTS];

/** @brief Pooled scratch buffers holding the entries of a PlayerListView */
static uint8_t *g_listBuffer[IE4_LIST_POOL_SLOTS];

/**
 * @brief Tells whether a view the pool still holds a buffer for is alive
 * @param view Address a slot was handed out to
 * @return int 1 while a PlayerListView still sits there, 0 once it is gone
 * @details The constructor the menus inline finishes by writing
 * IE4_LIST_VIEW_VTABLE at the view's `+0`, so a slot whose view no longer reads
 * back that word belongs to a menu object that has been freed and its buffer
 * can be handed to somebody else. A menu that is merely idle, one the player
 * left open under a submenu for instance, still reads back, so its entries are
 * never pulled out from under it.
 */
static int listViewAlive(int view) {
    return *(uint32_t *)view == IE4_LIST_VIEW_VTABLE;
}

/**
 * @brief Hands a view the scratch buffer its entries are written into
 * @param view PlayerListView asking for storage
 * @return uint8_t* Buffer of IE4_PLAYER_MAX entries, or NULL when none is left
 * @details A view keeps the same buffer across rebuilds, so a menu that reopens
 * does not churn the allocator. Buffers only ever move to another view once the
 * one holding them is dead; while every slot belongs to a live view the caller
 * falls back to the storage inside the view, which shows fewer players but
 * never hands out memory somebody else is reading.
 */
static uint8_t *listStorage(int view) {
    int i;

    for (i = 0; i < IE4_LIST_POOL_SLOTS; i++) {
        if (g_listBuffer[i] != 0 && g_listView[i] == view) {
            return g_listBuffer[i];
        }
    }

    for (i = 0; i < IE4_LIST_POOL_SLOTS; i++) {
        if (g_listBuffer[i] == 0) {
            g_listBuffer[i] = (uint8_t *)safeMalloc2(IE4_PLAYER_MAX * IE4_LIST_ENTRY_SIZE);
            if (g_listBuffer[i] == 0) {
                return 0;
            }
            g_listView[i] = view;
            return g_listBuffer[i];
        }
    }

    for (i = 0; i < IE4_LIST_POOL_SLOTS; i++) {
        if (!listViewAlive(g_listView[i])) {
            g_listView[i] = view;
            return g_listBuffer[i];
        }
    }

    return 0;
}

/**
 * @brief Appends one pointer to the table of a PlayerListView
 * @param view View to append to
 * @param entry Entry to point at, NULL to keep a team slot aligned
 */
static void listPush(int view, uint8_t *entry) {
    uint32_t count = *(uint32_t *)(view + 4);

    *(uint32_t *)(view + IE4_LIST_TABLE_OFFSET + 4 * count) = (uint32_t)entry;
    *(uint32_t *)(view + 4) = count + 1;
}

/**
 * @brief Fills one entry of a PlayerListView
 * @param entry Entry to fill
 * @param playerId Position id of the player
 * @param index Index the player sits at in the source list
 */
static void listWriteEntry(uint8_t *entry, uint32_t playerId, uint32_t index) {
    uint32_t group;

    if (index <= 4) {
        group = 0;
    } else if (index <= 15) {
        group = 1;
    } else {
        group = 2;
    }

    *(uint32_t *)entry = playerId;
    *(uint32_t *)(entry + 4) = group;
    *(uint32_t *)(entry + 8) = index;
}

__attribute__((optimize("no-tree-loop-distribute-patterns")))
int ie4PlayerListBuild(int view, int team, int filter, uint32_t excludeId) {
    OrderList *order;
    const uint32_t *ids;
    uint8_t *entry;
    void *holder;
    int manager;
    int database;
    int party;
    uint32_t total;
    uint32_t i;

    holder = *(void **)PARTY_OFFSET;
    if (holder == 0) {
        _assert_func(0, team);
    }
    manager = *((int *)holder + 4);
    if (manager == 0) {
        return 0;
    }

    holder = *(void **)IE4_CHARA_DB_HOLDER;
    if (holder == 0) {
        _assert_func(0, team);
    }
    database = *((int *)holder + 4);
    if (database == 0) {
        return 0;
    }

    // Not read from IE4_LIST_PARTY_OFFSET: callers store the party there once,
    // when the menu opens, and never refresh it, so the enlarged pointer table
    // has already grown over it by the time the list is rebuilt
    holder = *(void **)PARTY_OFFSET2;
    if (holder == 0) {
        _assert_func(0, team);
    }
    party = *((int *)holder + 4);
    if (party == 0) {
        return 0;
    }
    order = (OrderList *)(party + PARTY_ORDER_OFFSET);

    if (team != 0) {
        ids = &order->ids[IE4_ORDER_TEAM_BASE + IE4_TEAM_SIZE * team];
        total = IE4_TEAM_SIZE;
    } else {
        ids = &order->ids[0];
        total = order->count;
    }

    if ((filter & 0x10) != 0 && total > IE4_TEAM_SIZE) {
        total = IE4_TEAM_SIZE;
    }
    // The pointer table has room for (1804 - 8) / 4 entries and the scratch
    // buffer for IE4_PLAYER_MAX, so a count the save left too high freezes the
    // game rather than truncating a page; clamp it instead
    if (total > IE4_PLAYER_MAX) {
        total = IE4_PLAYER_MAX;
    }

    // A list the view can hold on its own never touches the pool, which leaves
    // the scratch buffers to the handful of views that really list a full roster
    entry = (uint8_t *)(view + IE4_LIST_STORAGE_OFFSET);
    if (total > IE4_LIST_LEGACY_MAX) {
        uint8_t *scratch = listStorage(view);

        if (scratch != 0) {
            entry = scratch;
        } else {
            // Every buffer belongs to a view that is still alive, so show what
            // fits rather than write over entries somebody else is reading
            total = IE4_LIST_LEGACY_MAX;
        }
    }

    if (filter != 0 && (filter & 0xF) != 0xF) {
        // Position filtered list: a rejected player leaves no hole
        for (i = 0; i < total; i++) {
            uint32_t playerId = ids[i];
            int record;
            int chara;
            int position;

            if (playerId == excludeId) {
                continue;
            }

            record = getPlayerOffset(manager, (int)playerId);
            if (record == 0) {
                continue;
            }

            chara = ie4CharaLookup(database, *(int *)(record + 8), 0);
            if (chara == 0) {
                continue;
            }

            listWriteEntry(entry, playerId, i);

            position = *(uint8_t *)(chara + 21);
            if (((filter & 1) != 0 && position == 1)
             || ((filter & 2) != 0 && position == 2)
             || ((filter & 4) != 0 && position == 3)
             || ((filter & 8) != 0 && position == 4)) {
                listPush(view, entry);
                entry += IE4_LIST_ENTRY_SIZE;
            }
        }

        return 1;
    }

    for (i = 0; i < total; i++) {
        uint32_t playerId = ids[i];
        int record;

        if (playerId == excludeId) {
            // The recruit list closes the hole, a team keeps its sixteen slots
            if (team != 0) {
                listPush(view, 0);
            }
            continue;
        }

        record = getPlayerOffset(manager, (int)playerId);
        if (record == 0 || ie4CharaLookup(database, *(int *)(record + 8), 0) == 0) {
            listPush(view, 0);
            continue;
        }

        listWriteEntry(entry, playerId, i);
        listPush(view, entry);
        entry += IE4_LIST_ENTRY_SIZE;
    }

    return 1;
}

/** @brief Sort keys of the entries of a view, one per entry, built per sort */
static uint32_t g_sortKey[IE4_PLAYER_MAX];

/**
 * @brief Reads the value a sort key names on one player
 * @param key Sort key, 1 to IE4_LIST_SORT_KEY_MAX
 * @param record Player record of the entry
 * @param name Name record of the entry, may be NULL
 * @param nickname Nickname record of the entry, may be NULL
 * @return uint32_t Value to sort on, 0 when the key names nothing
 */
static uint32_t sortValue(int key, int record, int name, int nickname) {
    switch (key) {
    case 1:
        return name != 0 ? *(uint16_t *)(name + 38) : 0;
    case 2:
        return nickname != 0 ? *(uint16_t *)(nickname + 38) : 0;
    case 3:
        return *(uint8_t *)(record + 22);
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
        return (uint32_t)ie4PlayerStatValue(record, key - 2);
    case 12:
        return *(uint16_t *)(record + 28);
    case 13:
        return *(uint16_t *)(record + 30);
    default:
        return 0;
    }
}

__attribute__((optimize("no-tree-loop-distribute-patterns")))
uint32_t ie4PlayerListSort(int view, int key) {
    void *holder;
    int manager;
    int charaBase;
    int database;
    int ascending;
    uint32_t count;
    uint32_t i;
    uint32_t j;

    holder = *(void **)PARTY_OFFSET;
    if (holder == 0) {
        _assert_func(0, key);
    }
    manager = *((int *)holder + 4);
    if (manager == 0) {
        return 0;
    }

    holder = *(void **)IE4_CHARA_BASE_HOLDER;
    if (holder == 0) {
        _assert_func(0, key);
    }
    charaBase = *((int *)holder + 4);
    if (charaBase == 0) {
        return 0;
    }

    holder = *(void **)IE4_CHARA_DB_HOLDER;
    if (holder == 0) {
        _assert_func(0, key);
    }
    database = *((int *)holder + 4);
    if (database == 0) {
        return 0;
    }

    count = *(uint32_t *)(view + 4);
    if (count > IE4_PLAYER_MAX) {
        count = IE4_PLAYER_MAX;
    }

    for (i = 0; i < count; i++) {
        g_sortKey[i] = 0;
    }

    for (i = 0; i < count; i++) {
        const uint8_t *entry = *(const uint8_t **)(view + IE4_LIST_TABLE_OFFSET + 4 * i);
        int record;
        int chara;
        int base;
        int nameDb;

        // A team view leaves a null here for every slot it could not resolve
        if (entry == 0) {
            continue;
        }

        record = getPlayerOffset(manager, (int)*(const uint32_t *)entry);
        if (record == 0) {
            continue;
        }

        chara = ie4CharaLookup(database, *(int *)(record + 8), 0);
        if (chara == 0) {
            continue;
        }

        base = ie4CharaBaseLookup(charaBase, *(int *)(chara + 4), 0);
        if (base == 0) {
            continue;
        }

        nameDb = *(int *)IE4_NAME_DB;
        if (nameDb == 0) {
            _assert_func(0, key);
        }

        g_sortKey[i] = sortValue(key, record,
                                 ie4NameLookup(nameDb, *(int *)(base + 8), 0, 1),
                                 ie4NameLookup(nameDb, *(int *)(base + 12), 0, 1));
    }

    // Names read alphabetically, everything else reads best first
    ascending = (key == 1 || key == 2);

    for (i = 0; i < count; i++) {
        for (j = i; j < count; j++) {
            uint32_t *left;
            uint32_t *right;
            uint32_t swap;
            int wanted;

            if (ascending) {
                wanted = g_sortKey[i] > g_sortKey[j];
            } else {
                wanted = g_sortKey[i] < g_sortKey[j];
            }
            if (!wanted) {
                continue;
            }

            left = *(uint32_t **)(view + IE4_LIST_TABLE_OFFSET + 4 * i);
            right = *(uint32_t **)(view + IE4_LIST_TABLE_OFFSET + 4 * j);

            // An unresolved slot has no payload to trade, leave the pair alone
            if (left == 0 || right == 0) {
                continue;
            }

            swap = left[0];
            left[0] = right[0];
            right[0] = swap;
            swap = left[1];
            left[1] = right[1];
            right[1] = swap;
            swap = left[2];
            left[2] = right[2];
            right[2] = swap;

            swap = g_sortKey[i];
            g_sortKey[i] = g_sortKey[j];
            g_sortKey[j] = swap;
        }
    }

    return 1;
}
