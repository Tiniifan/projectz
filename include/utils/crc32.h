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

#ifdef __cplusplus
}
#endif

#endif // UTILS_CRC32_H