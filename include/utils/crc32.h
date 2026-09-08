/**
 * @file utils_crc32.h
 * @brief CRC32 utility functions for data integrity checking
 * @details This header provides CRC32 calculation functionality with C++ compatibility
 */

#ifndef UTILS_CRC32_H
#define UTILS_CRC32_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate CRC32 checksum for given data
 * @param data Pointer to the data buffer to calculate CRC32 for
 * @param unk1 Unknown parameter - possibly data length or buffer size
 * @param unk2 Unknown parameter - possibly flags or additional configuration
 * @return CRC32 checksum value as integer
 * @note Parameters unk1 and unk2 need clarification for proper documentation
 * @warning Ensure data pointer is valid and not null before calling
 */
int getCrc32(uint8_t* data, int unk1, int unk2);

/**
 * @brief Calculate CRC32 checksum over a fixed length buffer
 * @param data Pointer to the data buffer to calculate CRC32 for
 * @param size Number of bytes to checksum
 * @return unsigned int Standard zlib CRC32 of the buffer, or 0 if data is null
 * or size is 0
 * @note Unlike getCrc32, which stops at the first null byte, this one honours
 * `size`. This is the variant used to protect .ie4 save files
 * @warning The checksum stored in a save file covers the *encrypted* payload
 */
unsigned int getCrc32Buffer(uint8_t* data, int size);

#ifdef __cplusplus
}
#endif

#endif // UTILS_CRC32_H