#ifndef IE4PLAYER_H
#define IE4PLAYER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Enum pour les positions des joueurs
typedef enum {
    UNPOSITION = 0,
    GOALKEEPER = 1,
    FORWARD = 2,
    MIDFIELDER = 3,
    DEFENDER = 4
} PlayerPosition;

// Enum pour les stats
typedef enum {
    KICK = 0,
    DRIBBLE = 1,
    TECHNIQUE = 2,
    BLOCK = 3,
    SPEED = 4,
    STAMINA = 5,
    CATCH = 6,
    LUCK = 7
} StatIndex;

int getStatIndex(const char* statName);
void getMirrorPairs(PlayerPosition position, int mirrors[8]);

#ifdef __cplusplus
}
#endif

#endif // IE4PLAYER_H