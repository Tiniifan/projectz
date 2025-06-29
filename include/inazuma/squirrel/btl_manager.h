#ifndef BTL_MANAGER_H
#define BTL_MANAGER_H

#include <stdint.h>
#include "../squirrel-2.2.3/squirrel/squirrel.h"

#ifdef __cplusplus
extern "C" {
#endif

// Constants
#define PLAYER_BLOCK_SIZE 252    // Size of each player data block in bytes
#define MAX_PLAYERS 16           // Maximum number of players supported

/**
 * @brief Gets the handle/offset for a rival party player
 * @param v Squirrel VM instance
 * @return SQInteger 1 (pushes player offset or null to stack)
 * 
 * This function searches for a rival player either by:
 * - String parameter: Player ID (searches by hash)
 * - Integer parameter: Player index (direct lookup)
 * 
 * @note Returns player memory offset if found, null otherwise
 */
SQInteger cmndGetRivalPartyHandle(HSQUIRRELVM v);

/**
 * @brief Gets a battle player by team and player ID
 * @param v Squirrel VM instance
 * @return SQInteger 1 (pushes player offset or null to stack)
 * 
 * Parameters expected on stack:
 * - Index 2: Team number (integer)
 * - Index 3: Player ID string
 * 
 * @note Uses CRC32 hash of player ID for lookup
 */
SQInteger cmndGetBtlPlayer(HSQUIRRELVM v);

/**
 * @brief Retrieves comprehensive information about a battle player
 * @param v Squirrel VM instance
 * @return SQInteger 1 (pushes player info table or null to stack)
 * 
 * Parameters expected on stack:
 * - Index 2: Player memory address (integer)
 * 
 * Returns a Squirrel table containing all player stats, attributes,
 * special moves, and other relevant battle information.
 * 
 * @note Player address should be obtained from cmndGetBtlPlayer or cmndGetRivalPartyHandle
 */
SQInteger cmndGetBtlPlayerInfo(HSQUIRRELVM v);

#ifdef __cplusplus
}
#endif

#endif // BTL_MANAGER_H