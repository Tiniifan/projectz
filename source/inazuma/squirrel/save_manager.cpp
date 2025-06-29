#include <stdint.h>
#include "../clib/string.h"
#include "../utils/crc32.h"
#include "../inazuma/ie4sq.h"
#include "../inazuma/ie4io.h"
#include "../inazuma/ie4std.h"
#include "../squirrel-2.2.3/squirrel/squirrel.h"
#include "../squirrel-2.2.3/squirrel/sqapi.h"
#include "../inazuma/squirrel/save_manager.h"
#include "../inazuma/squirrel/sql5.h"

// Global save file data instance, initialized to zero
SaveFileData g_saveFile = {0};

/**
 * Squirrel command to open a save data file for reading operations
 * @param v Squirrel VM instance
 * @return SQInteger Number of return values pushed to stack
 */
SQInteger cmndOpenSaveDataFile(HSQUIRRELVM v) {
    // Get number of arguments passed to the function
    SQInteger argc = sq_gettop(v);
   
    // Validate argument count (function name + 1 argument = 2 total)
    if (argc != 2) {
        return sq_throwerror(v, "openSaveDataFile requires 1 argument: filename");
    }
   
    // Extract filename string from Squirrel stack
    const char *filename;
    if (SQ_FAILED(sq_getstring(v, 2, &filename))) {
        return sq_throwerror(v, "Argument must be a string (filename)");
    }
   
    // Set up save data context
    int context = SAVE_DATA_OFFSET;
   
    // Close any previously opened save file
    if (g_saveFile.isOpen) {
        closeSaveFileHandle(g_saveFile.dataParam, g_saveFile.dataOffset);
        safeMemset((unsigned char*)&g_saveFile, sizeof(SaveFileData));
    }
   
    // Initialize data parameter with aligned context value
    g_saveFile.dataParam[0] = (context + 200) & 0xFFFFFFFC;
   
    // Initialize save file handle
    initSaveFileHandle((int)g_saveFile.handleBuffer, (unsigned int*)g_saveFile.dataParam);
   
    // Attempt to open the save data file
    bool success = openSaveDataFile(
        context,
        (char*)filename,
        &g_saveFile.dataOffset,
        &g_saveFile.dataSize,
        g_saveFile.handleBuffer
    );
   
    if (success) {
        // Mark save file as open
        g_saveFile.isOpen = true;
       
        // Create and populate return table with file information
        sq_newtable(v);
       
        // Add offset field to table
        sq_pushstring(v, "offset", -1);
        sq_pushinteger(v, g_saveFile.dataOffset);
        sq_newslot(v, -3, SQFalse);
       
        // Add size field to table
        sq_pushstring(v, "size", -1);
        sq_pushinteger(v, g_saveFile.dataSize);
        sq_newslot(v, -3, SQFalse);
    } else {
        // Return null on failure
        sq_pushnull(v);
    }
   
    return 1;
}

/**
 * Squirrel command to close the currently open save data file
 * @param v Squirrel VM instance
 * @return SQInteger Number of return values pushed to stack
 */
SQInteger cmndCloseSaveDataFile(HSQUIRRELVM v) {
    // Get number of arguments passed to the function
    SQInteger argc = sq_gettop(v);
   
    // Validate that no arguments are provided
    if (argc != 1) {
        return sq_throwerror(v, "closeSaveDataFile takes no arguments");
    }
   
    bool success = false;
   
    // Close save file if one is currently open
    if (g_saveFile.isOpen) {
        closeSaveFileHandle(g_saveFile.dataParam, g_saveFile.dataOffset);
        safeMemset((unsigned char*)&g_saveFile, sizeof(SaveFileData));
        success = true;
    }
   
    // Return success/failure status
    sq_pushbool(v, success ? SQTrue : SQFalse);
    return 1;
}

/**
 * Convert a single hexadecimal character to its numeric value
 * @param c Hexadecimal character ('0'-'9', 'A'-'F', 'a'-'f')
 * @return int Numeric value (0-15) or -1 for invalid character
 */
static int hexCharToValue(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1; // Invalid character
}

/**
 * Convert a hexadecimal string to byte array
 * @param hexStr Input hexadecimal string
 * @param outData Output pointer to allocated byte array
 * @param outSize Output size of the byte array
 * @return int 1 on success, 0 on failure
 */
static int hexStringToBytes(const char* hexStr, unsigned char** outData, int* outSize) {
    int len = safeStrlen(hexStr);
    
    // Verify that the length is even
    if (len % 2 != 0) {
        return 0; // Error: odd length
    }
    
    int dataSize = len / 2;
    int allocContext = SAVE_DATA_OFFSET + 200;
    
    // Allocate memory using custom allocator
    unsigned char* data = (unsigned char*)allocatorAlloc(allocContext, 1, dataSize, 2);
    if (!data) {
        return 0; // Allocation error
    }
    
    // Initialize allocated memory
    safeMemset(data, dataSize);
    
    // Convert each pair of hex characters to a byte
    for (int i = 0; i < dataSize; i++) {
        char highChar = hexStr[i * 2];
        char lowChar = hexStr[i * 2 + 1];
        
        // Convert each hex character to its value
        int highValue = hexCharToValue(highChar);
        int lowValue = hexCharToValue(lowChar);
        
        if (highValue == -1 || lowValue == -1) {
            // Free memory on error
            allocatorFree(&allocContext, (unsigned int)data, 1);
            return 0; // Error: non-hexadecimal character
        }
        
        // Combine the two values to form the byte
        data[i] = (unsigned char)((highValue << 4) | lowValue);
    }
    
    *outData = data;
    *outSize = dataSize;
    return 1; // Success
}

/**
 * Squirrel command to write hexadecimal data to a save data file
 * @param v Squirrel VM instance
 * @return SQInteger Number of return values pushed to stack
 */
SQInteger cmndWriteSaveDataFile(HSQUIRRELVM v) {
    // Get number of arguments passed to the function
    SQInteger argc = sq_gettop(v);
    if (argc != 3) {
        return sq_throwerror(v, "writeSaveDataFile requires 2 arguments: filename, hexdata");
    }
    
    // Extract filename string from Squirrel stack
    const char *filename;
    if (SQ_FAILED(sq_getstring(v, 2, &filename))) {
        return sq_throwerror(v, "First argument must be a string (filename)");
    }
    
    // Extract hexadecimal data string from Squirrel stack
    const char *hexString;
    if (SQ_FAILED(sq_getstring(v, 3, &hexString))) {
        return sq_throwerror(v, "Second argument must be a string (hex data)");
    }
    
    // Calculate data size (each pair of hex characters = 1 byte)
    int hexLen = safeStrlen(hexString);
    if (hexLen % 2 != 0) {
        return sq_throwerror(v, "Hex string must have even number of characters");
    }
    
    int dataSize = hexLen / 2;
    int context = SAVE_DATA_OFFSET + 200;
    
    // Allocate memory for the data buffer
    unsigned char *dataBuffer = (unsigned char*)allocatorAlloc(context, 0, dataSize, 0);
    if (!dataBuffer) {
        return sq_throwerror(v, "Failed to allocate memory for data buffer");
    }
    
    // Initialize buffer with zeros
    safeMemset(dataBuffer, dataSize);
    
    // Convert hexadecimal string to bytes
    for (int i = 0; i < dataSize; i++) {
        char hexByte[3] = {0};
        hexByte[0] = hexString[i * 2];
        hexByte[1] = hexString[i * 2 + 1];
        
        // Manual hex to byte conversion
        unsigned char byte = 0;
        for (int j = 0; j < 2; j++) {
            byte <<= 4;
            char c = hexByte[j];
            if (c >= '0' && c <= '9') {
                byte |= (c - '0');
            } else if (c >= 'A' && c <= 'F') {
                byte |= (c - 'A' + 10);
            } else if (c >= 'a' && c <= 'f') {
                byte |= (c - 'a' + 10);
            } else {
                // Invalid character - clean up and return error
                allocatorFree(&context, (unsigned int)dataBuffer, 0);
                return sq_throwerror(v, "Invalid hex character in data string");
            }
        }
        dataBuffer[i] = byte;
    }
    
    // Prepare variables for writeSaveDataFile (expects pointers)
    unsigned int dataOffset = (unsigned int)dataBuffer;
    
    // Write data to save file
    bool success = writeSaveDataFile(
        SAVE_DATA_OFFSET,
        (char*)filename,
        dataOffset,
        dataSize,
        0
    );
    
    // Free allocated memory
    allocatorFree(&context, (unsigned int)dataBuffer, 0);
    
    // Return the result
    sq_pushbool(v, success ? SQTrue : SQFalse);

    return 1;
}