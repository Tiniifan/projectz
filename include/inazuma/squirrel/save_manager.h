#ifndef SAVE_MANAGER_H
#define SAVE_MANAGER_H

#include <stdbool.h>
#include "../squirrel-2.2.3/squirrel/squirrel.h"

/**
 * @brief Structure to hold save file data and state information
 * @details Contains all necessary data for managing save file operations including
 * file handles, buffers, and metadata
 */
typedef struct {
    /** @brief Offset position in the save data file */
    unsigned int dataOffset;
    
    /** @brief Size of the save data in bytes */
    int dataSize;
    
    /** @brief Array of data parameters for save file operations */
    int dataParam[8];
    
    /** @brief Buffer for file handle operations */
    char handleBuffer[4];
    
    /** @brief Flag indicating if a save file is currently open */
    bool isOpen;
} SaveFileData;

/**
 * @brief Opens a save data file for reading/writing operations
 * @param v Squirrel VM instance
 * @details Squirrel function that takes 1 argument: filename (string)
 * @return SQInteger Returns a table with offset and size on success, null on failure
 * @note Automatically closes any previously opened save file
 */
SQInteger cmndOpenSaveDataFile(HSQUIRRELVM v);

/**
 * @brief Closes the currently open save data file
 * @param v Squirrel VM instance
 * @details Squirrel function that takes no arguments
 * @return SQInteger Returns boolean indicating success/failure
 * @note Safe to call even if no file is currently open
 */
SQInteger cmndCloseSaveDataFile(HSQUIRRELVM v);

/**
 * @brief Writes hexadecimal data to a save data file
 * @param v Squirrel VM instance
 * @details Squirrel function that takes 2 arguments: filename (string), hexdata (string)
 * @return SQInteger Returns boolean indicating write success/failure
 * @note Hex string must have even number of characters (each pair represents one byte)
 * @warning Automatically allocates and frees memory for the conversion process
 */
SQInteger cmndWriteSaveDataFile(HSQUIRRELVM v);

/**
 * @brief Global save file data instance
 * @details Maintains the current state of save file operations across function calls
 * @warning Should be properly initialized before use
 */
extern SaveFileData g_saveFile;

#endif // SAVE_MANAGER_H