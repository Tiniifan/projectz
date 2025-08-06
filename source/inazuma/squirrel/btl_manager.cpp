#include <stdint.h>
#include "../clib/string.h"
#include "../utils/crc32.h"
#include "../inazuma/ie4sq.h"
#include "../inazuma/ie4btl.h"
#include "../inazuma/ie4player.h"
#include "../squirrel-2.2.3/squirrel/squirrel.h"
#include "../squirrel-2.2.3/squirrel/sqapi.h"
#include "../inazuma/squirrel/btl_manager.h"
#include "../inazuma/squirrel/sql5.h"

SQInteger cmndGetRivalPartyHandle(HSQUIRRELVM v) {
    // Check if we have at least one parameter
    if (sq_gettop(v) < 2) {
        sq_pushnull(v);
        return 1;
    }

    // Get rival player base address from RAM
    int base = *reinterpret_cast<int*>(BTL_RIVAL_PARTY_OFFSET);
    int value = *reinterpret_cast<int*>(base + 0x10);
    int rivalPlayerBase = *reinterpret_cast<int*>(value + 40 + 8) + 4;
    
    int targetOffset = -1;

    // Check parameter type (string or integer)
    SQObjectType paramType = sq_gettype(v, 2);
    
    if (paramType == OT_STRING) {
        // Search by playerID hash
        const SQChar* playerID;
        sq_getstring(v, 2, &playerID);
        
        // Cast to match the original function signature (uint8_t*)
        int playerHash = getCrc32(reinterpret_cast<uint8_t*>(const_cast<char*>(playerID)), 0, 0);
        
        // Iterate through all player blocks to find matching hash
        for (int i = 0; i < MAX_PLAYERS; ++i) {
            int currentOffset = rivalPlayerBase + PLAYER_BLOCK_SIZE * i;
            
            // Check if playerID hash matches at offset +4
            if (*reinterpret_cast<int*>(currentOffset + 4) == playerHash) {
                // Verify validity (non-zero index at offset +0)
                if (*reinterpret_cast<int*>(currentOffset) != 0) {
                    targetOffset = currentOffset;
                    break;
                }
            }
        }
    } else if (paramType == OT_INTEGER) {
        // Search by player index
        SQInteger playerIndex;
        sq_getinteger(v, 2, &playerIndex);
        
        int currentOffset = rivalPlayerBase + PLAYER_BLOCK_SIZE * playerIndex;
        
        // Check validity (non-zero index at offset +0)
        if (*reinterpret_cast<int*>(currentOffset) != 0) {
            targetOffset = currentOffset;
        }
    }

    // Return player offset or null
    if (targetOffset != -1) {
        sq_pushinteger(v, targetOffset);
        return 1;
    } else {
        sq_pushnull(v);
        return 1;
    }
}

SQInteger cmndGetBtlPlayer(HSQUIRRELVM v) {
    // Check if we have the required parameters
    if (sq_gettop(v) < 3) {
        sq_pushnull(v);
        return 1;
    }

    // Get team parameter
    SQInteger team;
    if (SQ_FAILED(sq_getinteger(v, 2, &team))) {
        sq_pushnull(v);
        return 1;
    }

    // Get playerID parameter
    const SQChar* playerID;
    if (SQ_FAILED(sq_getstring(v, 3, &playerID))) {
        sq_pushnull(v);
        return 1;
    }

    // Calculate hash for playerID - cast to match original signature
    int hash = getCrc32(reinterpret_cast<uint8_t*>(const_cast<char*>(playerID)), 0, 0);

    // Cast hash to uint32_t to match the expected signature
    uint32_t* btlPlayer = getBtlPlayerOffset(reinterpret_cast<int*>(BTL_PLAYER_OFFSET), team, static_cast<uint32_t>(hash));

    if (btlPlayer) {
        sq_pushinteger(v, reinterpret_cast<int>(btlPlayer));
        return 1;
    } else {
        sq_pushnull(v);
        return 1;
    }
}

SQInteger cmndGetBtlPlayerInfo(HSQUIRRELVM v) {
    // Check if we have the required parameter
    if (sq_gettop(v) < 2) {
        sq_pushnull(v);
        return 1;
    }

    // Get player address
    SQInteger address;
    if (SQ_FAILED(sq_getinteger(v, 2, &address))) {
        sq_pushnull(v);
        return 1;
    }

    if (!address) {
        sq_pushnull(v);
        return 1;
    }

    const BattlePlayer* player = reinterpret_cast<BattlePlayer*>(address);

    // Create new table to store player information
    sq_newtable(v);

    // Helper macro to add integer field to table
    #define ADD_INT_FIELD(name, value) \
        sq_pushstring(v, _SC(name), -1); \
        sq_pushinteger(v, value); \
        sq_rawset(v, -3);

    // Basic information
    ADD_INT_FIELD("CharaparamID", player->CharaparamID);
    ADD_INT_FIELD("CharaBaseID", player->CharaBaseID);
    ADD_INT_FIELD("DescriptionID", player->DescriptionID);
    ADD_INT_FIELD("FightingSpiritID", player->FightingSpiritID);
    ADD_INT_FIELD("FightingSpiritMatchID", player->FightingSpiritMatchID);

    // Player attributes
    ADD_INT_FIELD("Element", player->Element);
    ADD_INT_FIELD("Position", player->Position);
    ADD_INT_FIELD("Level", player->Level);
    ADD_INT_FIELD("PlayerNumber", player->PlayerNumber);

    // Base stats
    ADD_INT_FIELD("FPBase", player->FPBase);
    ADD_INT_FIELD("TPBase", player->TPBase);
    ADD_INT_FIELD("KickBase", player->KickBase);
    ADD_INT_FIELD("DribbleBase", player->DribbleBase);
    ADD_INT_FIELD("TechniqueBase", player->TechniqueBase);
    ADD_INT_FIELD("BlockBase", player->BlockBase);
    ADD_INT_FIELD("SpeedBase", player->SpeedBase);
    ADD_INT_FIELD("StaminaBase", player->StaminaBase);
    ADD_INT_FIELD("CatchBase", player->CatchBase);
    ADD_INT_FIELD("LuckBase", player->LuckBase);

    // Growth stats
    ADD_INT_FIELD("FPGrow", player->FPGrow);
    ADD_INT_FIELD("TPGrow", player->TPGrow);
    ADD_INT_FIELD("KickGrow", player->KickGrow);
    ADD_INT_FIELD("DribbleGrow", player->DribbleGrow);
    ADD_INT_FIELD("TechniqueGrow", player->TechniqueGrow);
    ADD_INT_FIELD("BlockGrow", player->BlockGrow);
    ADD_INT_FIELD("SpeedGrow", player->SpeedGrow);
    ADD_INT_FIELD("StaminaGrow", player->StaminaGrow);
    ADD_INT_FIELD("CatchGrow", player->CatchGrow);
    ADD_INT_FIELD("LuckGrow", player->LuckGrow);

    // Current stats
    ADD_INT_FIELD("FPCurrent", player->FPCurrent);
    ADD_INT_FIELD("TPCurrent", player->TPCurrent);
    ADD_INT_FIELD("FPMax", player->FPMax);
    ADD_INT_FIELD("TPMax", player->TPMax);

    // Final stats
    ADD_INT_FIELD("KickFinal", player->KickFinal);
    ADD_INT_FIELD("DribbleFinal", player->DribbleFinal);
    ADD_INT_FIELD("TechniqueFinal", player->TechniqueFinal);
    ADD_INT_FIELD("BlockFinal", player->BlockFinal);
    ADD_INT_FIELD("SpeedFinal", player->SpeedFinal);
    ADD_INT_FIELD("StaminaFinal", player->StaminaFinal);
    ADD_INT_FIELD("CatchFinal", player->CatchFinal);
    ADD_INT_FIELD("LuckFinal", player->LuckFinal);

    // Special moves
    ADD_INT_FIELD("SpecialMove1ID", player->SpecialMove1ID);
    ADD_INT_FIELD("SpecialMove2ID", player->SpecialMove2ID);
    ADD_INT_FIELD("SpecialMove3ID", player->SpecialMove3ID);
    ADD_INT_FIELD("SpecialMove4ID", player->SpecialMove4ID);
    ADD_INT_FIELD("SpecialMove5ID", player->SpecialMove5ID);
    ADD_INT_FIELD("SpecialMove6ID", player->SpecialMove6ID);

    // Special move levels
    ADD_INT_FIELD("SpecialMove1Level", player->SpecialMove1Level);
    ADD_INT_FIELD("SpecialMove2Level", player->SpecialMove2Level);
    ADD_INT_FIELD("SpecialMove3Level", player->SpecialMove3Level);
    ADD_INT_FIELD("SpecialMove4Level", player->SpecialMove4Level);
    ADD_INT_FIELD("SpecialMove5Level", player->SpecialMove5Level);
    ADD_INT_FIELD("SpecialMove6Level", player->SpecialMove6Level);

    // Fighting spirit
    ADD_INT_FIELD("FightingSpiritAvailableID", player->FightingSpiritAvailableID);
    ADD_INT_FIELD("FightingSpiritLevel", player->FightingSpiritLevel);

    // Other fields
    ADD_INT_FIELD("SpecialMoveOffset", player->SpecialMoveOffset);
    ADD_INT_FIELD("SpecialMoveCount", player->SpecialMoveCount);
    ADD_INT_FIELD("Freedom", player->Freedom);
    ADD_INT_FIELD("PlayerGroup", player->PlayerGroup);
    ADD_INT_FIELD("ExperienceGrow", player->ExperienceGrow);
    ADD_INT_FIELD("StatVariance", player->StatVariance);
    ADD_INT_FIELD("Unk2", player->Unk2);
    ADD_INT_FIELD("TrainingUID", player->TrainingUID);
    ADD_INT_FIELD("TeamSide", player->TeamSide);
    ADD_INT_FIELD("PlayerIndexId", player->PlayerIndexId);

    // Unknown blocks (treat as integers)
    ADD_INT_FIELD("UnkBlock1", *reinterpret_cast<const int*>(&player->UnkBlock1[0]));
    ADD_INT_FIELD("UnkBlock2", *reinterpret_cast<const int*>(&player->UnkBlock2[0]));

    #undef ADD_INT_FIELD

    // Return the table (it's already on top of the stack)
    return 1;
}

SQInteger cmndTrainRivalPlayer(HSQUIRRELVM v) {
    // Vérifier le nombre minimum de paramètres (3 obligatoires)
    if (sq_gettop(v) < 4) {
        sq_pushbool(v, SQFalse);
        return 1;
    }

    // 1. Récupérer l'ID du joueur (string)
    const SQChar* playerID;
    if (SQ_FAILED(sq_getstring(v, 2, &playerID))) {
        sq_pushbool(v, SQFalse);
        return 1;
    }

    // 2. Récupérer le tableau d'équipements (8 éléments int)
    int equipments[8];
    safeMemset(reinterpret_cast<unsigned char*>(equipments), sizeof(equipments));
    
    if (sq_gettype(v, 3) == OT_ARRAY) {
        sq_push(v, 3);
        SQInteger arraySize = sq_getsize(v, -1);
        
        for (int i = 0; i < 8 && i < arraySize; i++) {
            sq_pushinteger(v, i);
            if (SQ_SUCCEEDED(sq_rawget(v, -2))) {
                SQInteger value;
                if (SQ_SUCCEEDED(sq_getinteger(v, -1, &value))) {
                    equipments[i] = (int)value;
                }
                sq_pop(v, 1);
            }
        }
        sq_pop(v, 1);
    }

    // 3. Récupérer le tableau des stats à entraîner (4 éléments max, string ou int)
    int trainStats[4];
    int usedStats[8];
    int trains[8];
    
    // Initialiser avec safeMemset au lieu de = {-1, -1, -1, -1}
    safeMemset(reinterpret_cast<unsigned char*>(trainStats), sizeof(trainStats));
    // Puis définir les valeurs à -1 manuellement
    for (int i = 0; i < 4; i++) {
        trainStats[i] = -1;
    }
    
    // Initialiser usedStats avec safeMemset au lieu de = {0}
    safeMemset(reinterpret_cast<unsigned char*>(usedStats), sizeof(usedStats));
    
    // Initialiser trains avec safeMemset
    safeMemset(reinterpret_cast<unsigned char*>(trains), sizeof(trains));
    
    int trainStatsCount = 0;
    
    if (sq_gettype(v, 4) == OT_ARRAY) {
        sq_push(v, 4);
        SQInteger arraySize = sq_getsize(v, -1);
        
        for (int i = 0; i < 4 && i < arraySize && trainStatsCount < 4; i++) {
            sq_pushinteger(v, i);
            if (SQ_SUCCEEDED(sq_rawget(v, -2))) {
                SQObjectType elemType = sq_gettype(v, -1);
                int statIndex = -1;
                
                if (elemType == OT_STRING) {
                    const SQChar* statName;
                    if (SQ_SUCCEEDED(sq_getstring(v, -1, &statName))) {
                        statIndex = getStatIndex(statName);
                    }
                } else if (elemType == OT_INTEGER) {
                    SQInteger value;
                    if (SQ_SUCCEEDED(sq_getinteger(v, -1, &value))) {
                        if (value >= 0 && value <= 7) {
                            statIndex = (int)value;
                        }
                    }
                }
                
                if (statIndex >= 0 && statIndex < 8 && !usedStats[statIndex]) {
                    trainStats[trainStatsCount] = statIndex;
                    usedStats[statIndex] = 1;
                    trainStatsCount++;
                }
                
                sq_pop(v, 1);
            }
        }
        sq_pop(v, 1);
    }

    // 4. Récupérer la position optionnelle (string ou int)
    PlayerPosition forcePosition = UNPOSITION;
    if (sq_gettop(v) >= 5) {
        SQObjectType posType = sq_gettype(v, 5);
        if (posType == OT_INTEGER) {
            SQInteger pos;
            if (SQ_SUCCEEDED(sq_getinteger(v, 5, &pos))) {
                if (pos >= 1 && pos <= 4) {
                    forcePosition = (PlayerPosition)pos;
                }
            }
        } else if (posType == OT_STRING) {
            const SQChar* posName;
            if (SQ_SUCCEEDED(sq_getstring(v, 5, &posName))) {
                if (safeStrcmp(posName, "Goalkeeper") == 0) forcePosition = GOALKEEPER;
                else if (safeStrcmp(posName, "Forward") == 0) forcePosition = FORWARD;
                else if (safeStrcmp(posName, "Midfielder") == 0) forcePosition = MIDFIELDER;
                else if (safeStrcmp(posName, "Defender") == 0) forcePosition = DEFENDER;
            }
        }
    }

    // Récupérer le joueur bataille
    int hash = getCrc32(reinterpret_cast<uint8_t*>(const_cast<char*>(playerID)), 0, 0);
    uint32_t* btlPlayerPtr = getBtlPlayerOffset(reinterpret_cast<int*>(BTL_PLAYER_OFFSET), 1, static_cast<uint32_t>(hash));
    
    if (!btlPlayerPtr) {
        sq_pushbool(v, SQFalse);
        return 1;
    }

    BattlePlayer* btlPlayer = reinterpret_cast<BattlePlayer*>(btlPlayerPtr);

    // Récupérer le handle du joueur rival
    int base = *reinterpret_cast<int*>(BTL_RIVAL_PARTY_OFFSET);
    int value = *reinterpret_cast<int*>(base + 0x10);
    int rivalPlayerBase = *reinterpret_cast<int*>(value + 40 + 8) + 4;
    
    int rivalHandle = -1;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        int currentOffset = rivalPlayerBase + PLAYER_BLOCK_SIZE * i;
        if (*reinterpret_cast<int*>(currentOffset + 4) == hash) {
            if (*reinterpret_cast<int*>(currentOffset) != 0) {
                rivalHandle = currentOffset;
                break;
            }
        }
    }

    if (rivalHandle == -1) {
        sq_pushbool(v, SQFalse);
        return 1;
    }

    // Déterminer la position à utiliser
    PlayerPosition position = (forcePosition != UNPOSITION) ? forcePosition : (PlayerPosition)btlPlayer->Position;

    // Récupérer les stats actuelles
    uint16_t stats[8];
    stats[0] = btlPlayer->KickFinal;
    stats[1] = btlPlayer->DribbleFinal;
    stats[2] = btlPlayer->TechniqueFinal;
    stats[3] = btlPlayer->BlockFinal;
    stats[4] = btlPlayer->SpeedFinal;
    stats[5] = btlPlayer->StaminaFinal;
    stats[6] = btlPlayer->CatchFinal;
    stats[7] = btlPlayer->LuckFinal;

    // Soustraire les équipements des stats actuelles
    for (int i = 0; i < 8; i++) {
        stats[i] -= equipments[i];
    }

    // Calculer les modifications d'entraînement
    int mirrors[8];
    getMirrorPairs(position, mirrors);

    // Vérifier les conflits avec les stats miroirs
    for (int i = 0; i < trainStatsCount; i++) {
        int stat = trainStats[i];
        int mirrorStat = mirrors[stat];
        
        for (int j = 0; j < trainStatsCount; j++) {
            if (j != i && trainStats[j] == mirrorStat) {
                trainStats[i] = -1;
                break;
            }
        }
    }

    // Appliquer le bonus de liberté à la première stat valide
    int freedom = btlPlayer->Freedom;
    int firstStatBonusApplied = 0;
    
    for (int i = 0; i < trainStatsCount; i++) {
        int stat = trainStats[i];
        if (stat >= 0) {
            if (!firstStatBonusApplied) {
                trains[stat] += freedom;
                firstStatBonusApplied = 1;
            }
            
            int mirrorStat = mirrors[stat];
            if (mirrorStat >= 0) {
                int mirrorValue = stats[mirrorStat] - 1;
                if (mirrorValue > 0) {
                    trains[stat] += mirrorValue;
                    trains[mirrorStat] -= mirrorValue;
                }
            }
        }
    }

    // Écrire les nouvelles valeurs en mémoire
    uint16_t* finalStats[8];
    finalStats[0] = &btlPlayer->KickFinal;
    finalStats[1] = &btlPlayer->DribbleFinal;
    finalStats[2] = &btlPlayer->TechniqueFinal;
    finalStats[3] = &btlPlayer->BlockFinal;
    finalStats[4] = &btlPlayer->SpeedFinal;
    finalStats[5] = &btlPlayer->StaminaFinal;
    finalStats[6] = &btlPlayer->CatchFinal;
    finalStats[7] = &btlPlayer->LuckFinal;

    for (int i = 0; i < 8; i++) {
        int finalStat = stats[i] + trains[i] + equipments[i];
        if (finalStat < 0) finalStat = 0;
        if (finalStat > 65535) finalStat = 65535;
        
        *finalStats[i] = (uint16_t)finalStat;
        *reinterpret_cast<int16_t*>(rivalHandle + 32 + (i * 2)) = (int16_t)trains[i];
    }

    sq_pushbool(v, SQTrue);
    return 1;
}