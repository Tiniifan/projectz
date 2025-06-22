#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdint.h>
#include "../squirrel-2.2.3/squirrel/squirrel.h"

#ifdef __cplusplus
extern "C" {
#endif

// Constantes pour les types de données
#define BYTE   0
#define CHAR   1
#define SHORT  2
#define USHORT 3
#define INT    4
#define UINT   5
#define FLOAT  6
#define STRING 7

/**
 * @brief Fonction de lecture mémoire pour Squirrel
 * @param v La VM Squirrel
 * @return Nombre de valeurs retournées sur la pile
 */
SQInteger cmndMemoryRead(HSQUIRRELVM v);

#ifdef __cplusplus
}
#endif

#endif // MEMORY_MANAGER_H