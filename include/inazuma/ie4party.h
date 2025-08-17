#ifndef IE4PARTY_H
#define IE4PARTY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Memory offset for party player data
 * @details Base memory address where party player data is stored
 */
#define PARTY_OFFSET 0x5B0968

#define PARTY_OFFSET2 0x5B0970

/**
 * @brief Retrieves battle player data offset based on team and hash
 * @param base Base
 * @param playerIndex playerIndex
 * @return playerIndex
 * @note Returns NULL if player not found or invalid parameters
 */
int getPlayerOffset(int base, int playerIndex);

#ifdef __cplusplus
}
#endif

#endif // IE4PARTY_H