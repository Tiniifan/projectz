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

/**
 * @brief Changes the coach of a team for the current match
 * @param v Squirrel VM instance
 * @return SQInteger 1 (pushes true/false to stack)
 *
 * Parameters expected on stack:
 * - Index 2: Team number (integer, 0 or 1)
 * - Index 3: Coach identifier (string ID like "iim0005" or integer number like 5)
 *
 * @note The team does not need to own that coach. The coach bonus is applied
 * right away; works from BtlSetting as well as during the match
 */
SQInteger cmndBtlSetCoach(HSQUIRRELVM v);

/**
 * @brief Removes the coach of a team for the current match
 * @param v Squirrel VM instance
 * @return SQInteger 1 (pushes true/false to stack)
 *
 * Parameters expected on stack:
 * - Index 2: Team number (integer, 0 or 1)
 *
 * @note The coach bonus is dropped right away. Picking a coach in the pause
 * menu brings one back
 */
SQInteger cmndBtlRemoveCoach(HSQUIRRELVM v);

/**
 * @brief Changes the emblem of a team for the current match only
 * @param v Squirrel VM instance
 * @return SQInteger 1 (pushes true/false to stack)
 *
 * Parameters expected on stack:
 * - Index 2: Team number (integer, 0 or 1)
 * - Index 3: Emblem identifier (integer number like 12, or a file name like
 *   "emb0012", "ems0012" or "emf0012", only its digits are used)
 *
 * @warning The match loads its emblem images once, so call this from InitBtl
 * or BtlSetting; a later call only reaches what reads the emblem afterwards
 */
SQInteger cmndBtlSetEmblem(HSQUIRRELVM v);

/**
 * @brief Changes the team level of a team for the current match
 * @param v Squirrel VM instance
 * @return SQInteger 1 (pushes true/false to stack)
 *
 * Parameters expected on stack:
 * - Index 2: Team number (integer, 0 or 1)
 * - Index 3: Team level (integer, 0 to 255)
 *
 * @note This is the level shown on the versus screen before kick-off. The game
 * computes it once at the start of the match, so call this from InitBtl or
 * BtlSetting, after changing the members of the team
 */
SQInteger cmndBtlSetTeamLevel(HSQUIRRELVM v);

#ifdef __cplusplus
}
#endif

#endif // BTL_MANAGER_H