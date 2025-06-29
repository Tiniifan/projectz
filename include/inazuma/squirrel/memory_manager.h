#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H
#include <stdint.h>
#include "../squirrel-2.2.3/squirrel/squirrel.h"
#ifdef __cplusplus
extern "C" {
#endif
// Data type constants
#define BYTE   0
#define CHAR   1
#define SHORT  2
#define USHORT 3
#define INT    4
#define UINT   5
#define FLOAT  6
#define STRING 7
/**
 * @brief Memory read function for Squirrel
 * @param v The Squirrel VM
 * @return Number of values returned on the stack
 */
SQInteger cmndMemoryRead(HSQUIRRELVM v);

/**
 * @brief Memory write function for Squirrel
 * @param v The Squirrel VM
 * @return Number of values returned on the stack
 */
SQInteger cmndMemoryWrite(HSQUIRRELVM v);
#ifdef __cplusplus
}
#endif
#endif // MEMORY_MANAGER_H