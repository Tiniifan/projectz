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

// Management
#define BTL_SLOT_SIZE 164       // 0xA4
#define BTL_TEAM_STRIDE 2624    // step between Team 0 and Team 1
#define BTL_PLAYERS_BASE 4      // offset of the player array within the container
#define BTL_COUNT_BASE 5252     // meter table offset (2 integers)

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


SQInteger cmndTrainRivalPlayer(HSQUIRRELVM v);


SQInteger cmndBtlGetUniform(HSQUIRRELVM v);


SQInteger cmndBtlGetUniformNum(HSQUIRRELVM v);


SQInteger cmndBtlIsAwayKit(HSQUIRRELVM v);

/**
 * @brief Removes a player from a battle team
 * @param v Squirrel VM instance
 * @return SQInteger 1 (pushes true/false to stack)
 *
 * Parameters expected on stack:
 * - Index 2: Team number (integer, 0 or 1)
 * - Index 3: Player identifier (string ID or integer position)
 *
 * @note Shifts subsequent players left and decrements team player count
 */
SQInteger cmndBtlRemoveMember(HSQUIRRELVM v);

/**
 * @brief Gets the current number of players in a battle team
 * @param v Squirrel VM instance
 * @return SQInteger 1 (pushes player count or null to stack)
 *
 * Parameters expected on stack:
 * - Index 2: Team number (integer, 0 or 1)
 *
 * @note Returns null if team manager or container pointers are invalid
 */
SQInteger cmndBtlGetTeamPlayerCount(HSQUIRRELVM v);

/**
 * @brief Removes all players from a battle team
 * @param v Squirrel VM instance
 * @return SQInteger 1 (pushes true/false to stack)
 *
 * Parameters expected on stack:
 * - Index 2: Team number (integer, 0 or 1)
 *
 * @note Clears all occupied player slots and resets player count to zero
 */
SQInteger cmndBtlClearPlayersFromTeam(HSQUIRRELVM v);

#ifdef __cplusplus
}
#endif

#endif // BTL_MANAGER_H