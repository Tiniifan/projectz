#include <stdint.h>
#include "../clib/string.h"
#include "../inazuma/ie4sq.h"
#include "../squirrel-2.2.3/squirrel/squirrel.h"
#include "../squirrel-2.2.3/squirrel/sqapi.h"
#include "../inazuma/squirrel/memory_manager.h"
#include "../inazuma/squirrel/sql5.h"

#define BYTE   0
#define CHAR   1
#define SHORT  2
#define USHORT 3
#define INT    4
#define UINT   5
#define FLOAT  6
#define STRING 7

SQInteger cmndMemoryRead(HSQUIRRELVM v) {
    // Check minimum number of arguments (at least 2)
    if (sq_gettop(v) < 3) { // +1 for 'this' in the stack
        sq_pushnull(v);
        return 1;
    }
    
    SQInteger valueType = 0;
    SQInteger memoryAddress = 0;
    
    // Get the first argument (data type)
    if (sq_gettype(v, 2) == OT_STRING) {
        // If it's a string, convert to numeric type
        const SQChar* fieldStr;

        if (SQ_SUCCEEDED(sq_getstring(v, 2, &fieldStr))) {
            if (safeStrcmp((char*)fieldStr, "b") == 0) valueType = BYTE;
            else if (safeStrcmp((char*)fieldStr, "c") == 0) valueType = CHAR;
            else if (safeStrcmp((char*)fieldStr, "h") == 0) valueType = SHORT;
            else if (safeStrcmp((char*)fieldStr, "H") == 0) valueType = USHORT;
            else if (safeStrcmp((char*)fieldStr, "i") == 0) valueType = INT;
            else if (safeStrcmp((char*)fieldStr, "I") == 0) valueType = UINT;
            else if (safeStrcmp((char*)fieldStr, "f") == 0) valueType = FLOAT;
            else if (safeStrcmp((char*)fieldStr, "s") == 0) valueType = STRING;
            else {
                sq_pushnull(v);
                return 1;
            }
        } else {
            sq_pushnull(v);
            return 1;
        }
    } else if (sq_gettype(v, 2) == OT_INTEGER) {
        // If it's an integer, use it directly
        if (SQ_FAILED(sq_getinteger(v, 2, &valueType))) {
            sq_pushnull(v);
            return 1;
        }
    } else {
        sq_pushnull(v);
        return 1;
    }
    
    // Get the second argument (memory address)
    if (SQ_FAILED(sq_getinteger(v, 3, &memoryAddress))) {
        sq_pushnull(v);
        return 1;
    }
    
    // Security check for memory address
    if (memoryAddress == 0) {
        sq_pushnull(v);
        return 1;
    }
    
    // Read memory according to type
    switch (valueType) {
        case BYTE: {
            int8_t value = *((int8_t*)memoryAddress);
            sq_pushinteger(v, (SQInteger)value);
            return 1;
        }
        case CHAR: {
            char value = *((char*)memoryAddress);
            SQChar str[2] = { value, 0 };
            sq_pushstring(v, str, 1);
            return 1;
        }
        case SHORT: {
            int16_t value = *((int16_t*)memoryAddress);
            sq_pushinteger(v, (SQInteger)value);
            return 1;
        }
        case USHORT: {
            uint16_t value = *((uint16_t*)memoryAddress);
            sq_pushinteger(v, (SQInteger)value);
            return 1;
        }
        case INT: {
            int32_t value = *((int32_t*)memoryAddress);
            sq_pushinteger(v, (SQInteger)value);
            return 1;
        }
        case UINT: {
            uint32_t value = *((uint32_t*)memoryAddress);
            sq_pushinteger(v, (SQInteger)value);
            return 1;
        }
        case FLOAT: {
            float value = *((float*)memoryAddress);
            sq_pushfloat(v, (SQFloat)value);
            return 1;
        }
        case STRING: {
            SQChar* ptr = (SQChar*)memoryAddress;

            // Security limit
            int maxLen = 1024; 

            int len = 0;

            while (len < maxLen && ptr[len] != '\0') len++;
            
            // String too long or not null-terminated            
            if (len == maxLen) {
                sq_pushnull(v);
                return 1;
            }
            
            sq_pushstring(v, ptr, len);
            return 1;
        }
        default:
            sq_pushnull(v);
            return 1;
    }
}