#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H

#include <stdint.h>
#include "../squirrel-2.2.3/squirrel/squirrel.h"

#ifdef __cplusplus
extern "C" {
#endif

SQInteger cmndGetLv(HSQUIRRELVM v);

SQInteger cmndGetPartyLv(HSQUIRRELVM v);

#ifdef __cplusplus
}
#endif

#endif // PLAYER_MANAGER_H