#ifndef SQL5_H
#define SQL5_H

#include "../squirrel-2.2.3/squirrel/squirrel.h"

#ifdef __cplusplus
extern "C" {
#endif

void sq_pushnull_L5(HSQUIRRELVM v);
void sq_pushtrue_L5(HSQUIRRELVM v);
SQInteger sq_getinteger_cast_L5(HSQUIRRELVM v, int argIndex);
void sq_pushstring_L5(HSQUIRRELVM v, int argIndex);
SQBool sq_gettypetag_L5(HSQUIRRELVM v, int argIndex);
const SQChar* sq_getstring_L5(HSQUIRRELVM v, int argIndex, int unk1, int unk2);
SQInteger sq_copy_stack_L5(HSQUIRRELVM v);
void sq_loadargs_L5(HSQUIRRELVM source, HSQUIRRELVM destination);

#ifdef __cplusplus
}
#endif

#endif // SQL5_H