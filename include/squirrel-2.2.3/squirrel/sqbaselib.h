/*
	see copyright notice in squirrel.h
*/

#ifndef _SQBASELIB_H_
#define _SQBASELIB_H_

// Inclusions nécessaires pour les types utilisés dans les déclarations de fonctions
#include "squirrel.h"   // Pour HSQUIRRELVM, SQUIRREL_API, SQChar
#include "sqobject.h"   // Pour SQObjectPtr

#ifdef __cplusplus
extern "C" {
#endif

/*
* Tente de convertir une chaîne de caractères en un objet nombre Squirrel (entier ou flottant).
*
* @param s La chaîne de caractères source à convertir.
* @param res L'objet SQObjectPtr qui recevra le résultat numérique.
* @return 'true' si la conversion a réussi, 'false' sinon.
*/
SQUIRREL_API bool str2num(const SQChar *s, SQObjectPtr &res);

/*
* Enregistre toutes les fonctions et les délégués par défaut de la bibliothèque de base
* dans la machine virtuelle Squirrel spécifiée.
*
* @param v Le handle de la machine virtuelle Squirrel.
*/
SQUIRREL_API void sq_base_register(HSQUIRRELVM v);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*_SQBASELIB_H_*/