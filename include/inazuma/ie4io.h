#ifndef IE4IO_H
#define IE4IO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Memory offset for save data operations
 * @details Base memory address used as context for save file operations
 * and memory allocation within the game's save system
 */
#define SAVE_DATA_OFFSET 0x5C4EE8

/**
 * @brief Opens a save data file for reading operations
 * @param contextOffset Memory context offset for save operations
 * @param filename Name of the save file to open
 * @param dataOffset Output pointer to store the data offset within the file
 * @param dataSize Output pointer to store the size of the data in bytes
 * @param unk Unknown parameter, likely a buffer or handle identifier
 * @return bool True if file opened successfully, false otherwise
 * @note The dataOffset and dataSize parameters are modified to contain file information
 * @warning Ensure filename is a valid null-terminated string
 */
bool openSaveDataFile(int contextOffset, char *filename, unsigned int *dataOffset, int *dataSize, char *unk);

/**
 * @brief Writes data to a save data file
 * @param contextOffset Memory context offset for save operations
 * @param filename Name of the save file to write to
 * @param dataOffset Memory offset where the data to write is located
 * @param dataSize Size of the data to write in bytes
 * @param unk Unknown parameter, possibly file mode or flags
 * @return bool True if data written successfully, false otherwise
 * @note The file will be created if it doesn't exist
 * @warning Ensure dataOffset points to valid allocated memory of at least dataSize bytes
 */
bool writeSaveDataFile(int contextOffset, char *filename, unsigned int dataOffset, int dataSize, int unk);

/**
 * @brief Initializes a save file handle for subsequent operations
 * @param unk Unknown parameter, likely a handle or buffer identifier
 * @param dataParam Pointer to data parameters array for save operations
 * @return int Status code indicating success/failure of initialization
 * @note Must be called before performing save file operations
 * @warning dataParam must point to a valid array of sufficient size
 */
int initSaveFileHandle(int unk, unsigned int *dataParam);

/**
 * @brief Closes an open save file handle and cleans up resources
 * @param dataParam Pointer to data parameters array used in save operations
 * @param dataOffset Data offset that was used during file operations
 * @return int Status code indicating success/failure of cleanup
 * @note Should be called to properly release resources after save operations
 * @warning Calling this function invalidates the associated file handle
 */
int closeSaveFileHandle(int *dataParam, int dataOffset);

#ifdef __cplusplus
}
#endif

#endif // IE4IO_H