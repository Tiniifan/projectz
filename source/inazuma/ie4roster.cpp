#include <stdint.h>
#include "../clib/string.h"
#include "../inazuma/ie4party.h"
#include "../inazuma/ie4roster.h"

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
            record = (uint8_t *)ie4PlayerRecordInit(record);
            if (record == 0) {
                continue;
            }

            deserializePlayerData((int)(record + 4), (const int *)block);
            playerId = *(uint32_t *)(record + 4);

            ie4PlayerBuildStats(stats, record[22], playerId);
            for (k = 0; k < 10; k++) {
                *(uint16_t *)(record + 208 + 2 * k) = (uint16_t)stats[k];
            }

            ie4PlayerRefreshEquipment(manager, playerId, 1);
            ie4PlayerRefreshMoves(manager, playerId, 1);

            if (*(uint32_t *)(record + 52) != 0) {
                ie4PlayerRegisterAvatar(record[56], playerId);
            }

            bitmapSet(pool->bitmap, slot, 1);
            pool->serial++;
            pool->used++;
        }
    }

    *outSize = size;
    return 1;
}

__attribute__((optimize("no-tree-loop-distribute-patterns")))
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

    for (i = 0; i < recruited; i++) {
        ie4OrderListAppend((int)&IE4_ORDER_HANDLE, readU32(data + 4 * i), -1, 0);
    }

    // Team slots hold ids that are already in the recruit list, so they are
    // rewritten from it and land on their new index whatever the file said
    for (i = 0; i < IE4_TEAM_SLOT_COUNT; i++) {
        int found = ie4OrderListFind(order, 0, readU32(data + 4 * (recruited + i)));

        if (found >= 0) {
            order->ids[IE4_PLAYER_MAX + i] = order->ids[found];
        }
    }

    trailer = data + size;
    if (trailer + 4 > end) {
        return 0;
    }
    if (readSectionEnd(trailer) != IE4_SECTION_END) {
        return 0;
    }

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

/** @brief Build the slot was last handed out on, oldest slot loses its buffer */
static uint32_t g_listUsed[IE4_LIST_POOL_SLOTS];

/** @brief Number of builds served so far, stamped on the slot that serves one */
static uint32_t g_listClock;

/**
 * @brief Hands a view the scratch buffer its entries are written into
 * @param view PlayerListView asking for storage
 * @return uint8_t* Buffer of IE4_PLAYER_MAX entries, or NULL when none is left
 * @details A view keeps the same buffer across rebuilds, so a menu that reopens
 * does not churn the allocator. Once every slot is taken the buffer of the view
 * that was built longest ago is handed over, which never takes one away from a
 * view the player is still browsing since browsing rebuilds it.
 */
static uint8_t *listStorage(int view) {
    int oldest = 0;
    int i;

    g_listClock++;

    for (i = 0; i < IE4_LIST_POOL_SLOTS; i++) {
        if (g_listBuffer[i] != 0 && g_listView[i] == view) {
            g_listUsed[i] = g_listClock;
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
            g_listUsed[i] = g_listClock;
            return g_listBuffer[i];
        }
    }

    for (i = 1; i < IE4_LIST_POOL_SLOTS; i++) {
        if (g_listUsed[i] < g_listUsed[oldest]) {
            oldest = i;
        }
    }

    g_listView[oldest] = view;
    g_listUsed[oldest] = g_listClock;
    return g_listBuffer[oldest];
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

    entry = listStorage(view);
    if (entry == 0) {
        // Out of scratch buffers: fall back to the storage the game embedded in
        // the view, which cannot hold more than IE4_LIST_LEGACY_MAX entries
        entry = (uint8_t *)(view + IE4_LIST_STORAGE_OFFSET);
        if (total > IE4_LIST_LEGACY_MAX) {
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
