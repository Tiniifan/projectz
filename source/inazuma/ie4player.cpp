#include "../clib/string.h"
#include "../inazuma/ie4player.h"

int getStatIndex(const char* statName) {
    if (safeStrcmp(statName, "Kick") == 0) return KICK;
    if (safeStrcmp(statName, "Dribble") == 0) return DRIBBLE;
    if (safeStrcmp(statName, "Technique") == 0) return TECHNIQUE;
    if (safeStrcmp(statName, "Block") == 0) return BLOCK;
    if (safeStrcmp(statName, "Speed") == 0) return SPEED;
    if (safeStrcmp(statName, "Stamina") == 0) return STAMINA;
    if (safeStrcmp(statName, "Catch") == 0) return CATCH;
    if (safeStrcmp(statName, "Luck") == 0) return LUCK;

    return -1;
}

void getMirrorPairs(PlayerPosition position, int mirrors[8]) {
    mirrors[0] = -1;
    mirrors[1] = -1;
    mirrors[2] = -1;
    mirrors[3] = -1;
    mirrors[4] = -1;
    mirrors[5] = -1;
    mirrors[6] = -1;
    mirrors[7] = -1;
   
    switch (position) {
        case FORWARD:
            mirrors[KICK] = TECHNIQUE;
            mirrors[TECHNIQUE] = KICK;
            mirrors[DRIBBLE] = BLOCK;
            mirrors[BLOCK] = DRIBBLE;
            mirrors[SPEED] = STAMINA;
            mirrors[STAMINA] = SPEED;
            mirrors[CATCH] = LUCK;
            mirrors[LUCK] = CATCH;
            break;
           
        case MIDFIELDER:
            mirrors[KICK] = CATCH;
            mirrors[CATCH] = KICK;
            mirrors[DRIBBLE] = TECHNIQUE;
            mirrors[TECHNIQUE] = DRIBBLE;
            mirrors[BLOCK] = LUCK;
            mirrors[LUCK] = BLOCK;
            mirrors[SPEED] = STAMINA;
            mirrors[STAMINA] = SPEED;
            break;
           
        case DEFENDER:
            mirrors[KICK] = CATCH;
            mirrors[CATCH] = KICK;
            mirrors[DRIBBLE] = LUCK;
            mirrors[LUCK] = DRIBBLE;
            mirrors[TECHNIQUE] = BLOCK;
            mirrors[BLOCK] = TECHNIQUE;
            mirrors[SPEED] = STAMINA;
            mirrors[STAMINA] = SPEED;
            break;
           
        case GOALKEEPER:
            mirrors[KICK] = LUCK;
            mirrors[LUCK] = KICK;
            mirrors[DRIBBLE] = BLOCK;
            mirrors[BLOCK] = DRIBBLE;
            mirrors[TECHNIQUE] = CATCH;
            mirrors[CATCH] = TECHNIQUE;
            mirrors[SPEED] = STAMINA;
            mirrors[STAMINA] = SPEED;
            break;
           
        default:
            break;
    }
}