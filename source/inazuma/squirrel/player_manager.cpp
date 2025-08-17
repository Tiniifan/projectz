#include <stdint.h>
#include "../clib/string.h"
#include "../utils/crc32.h"
#include "../inazuma/ie4sq.h"
#include "../inazuma/ie4party.h"
#include "../squirrel-2.2.3/squirrel/squirrel.h"
#include "../squirrel-2.2.3/squirrel/sqapi.h"
#include "../inazuma/squirrel/player_manager.h"
#include "../inazuma/squirrel/sql5.h"

SQInteger cmndGetLv(HSQUIRRELVM v) {
    if (sq_gettop(v) < 2) {
        sq_pushnull(v);
        return 1;
    }
    
    int base = *reinterpret_cast<int*>(PARTY_OFFSET);
    int partyOffset = *reinterpret_cast<int*>(base + 0x10);
    if (partyOffset == 0) {
        sq_pushnull(v);
        return 1;
    }

    SQInteger playerIndex;
    if (SQ_FAILED(sq_getinteger(v, 2, &playerIndex))) {
        sq_pushnull(v);
        return 1;
    }

    SQInteger playerOffset = getPlayerOffset(partyOffset, playerIndex);
    if (playerOffset == 0) {
        sq_pushnull(v);
        return 1;
    }
    
    SQInteger level = *(char *)(playerOffset + 0x16);
    
    sq_pushinteger(v, level);
    return 1;
}

SQInteger cmndGetPartyLv(HSQUIRRELVM v) {
    int nbPlayer = 0;
    int sumLevel = 0;

    int baseA1 = *reinterpret_cast<int *>(PARTY_OFFSET2);
    int a1 = *reinterpret_cast<int*>(baseA1 + 0x10); 

    int basePartyOffset = *reinterpret_cast<int*>(PARTY_OFFSET);
    int partyOffset = *reinterpret_cast<int*>(basePartyOffset + 0x10);   

    int v5 = a1 + 336;
    int *v6 = reinterpret_cast<int *>(v5 + 4);
    int *v7 = reinterpret_cast<int *>(v5 + 644);

    if (v6 != v7) {
        do {
            int playerIndex = *v6;

            SQInteger playerOffset = getPlayerOffset(partyOffset, playerIndex);

            if (playerOffset != 0) {
                unsigned char playerLevel = *reinterpret_cast<unsigned char *>(playerOffset + 0x16);

                nbPlayer++;
                sumLevel += static_cast<int>(playerLevel);
            }
        } while (++v6 != v7);
    }

    SQInteger avgLevel = (nbPlayer > 0) ? (sumLevel / nbPlayer) : 0;

    sq_pushinteger(v, avgLevel);

    return 1;
}

