#ifndef IE4SQ_H
#define IE4SQ_H

#include "../squirrel-2.2.3/squirrel/squirrel.h"

#ifdef __cplusplus
extern "C" {
#endif

// VM setup
void createSquirrelVM(int *a1, int *a2);
int register_global_func(HSQUIRRELVM v, const char *fname, int adress);

// VM management
HSQUIRRELVM IE4RegisterSquirrelFunc(int gameContext, HSQUIRRELVM squirrelVM);
HSQUIRRELVM getSquirrelVM(HSQUIRRELVM v);

// Native functions
SQInteger cmndRandom(HSQUIRRELVM v);
SQInteger cmndSetLv(HSQUIRRELVM v);
SQInteger cmndGetHash(HSQUIRRELVM v);
SQInteger cmndAddParty(HSQUIRRELVM v);
SQInteger cmndGetPosry(HSQUIRRELVM v);
SQInteger cmndGpIsMax(HSQUIRRELVM v);
SQInteger cmndKeyInput(HSQUIRRELVM v);
SQInteger cmndSetPosry(HSQUIRRELVM v);
SQInteger sysEventStep(HSQUIRRELVM v);
SQInteger cmndLoadImage(HSQUIRRELVM v);
SQInteger sysCacheClear(HSQUIRRELVM v);
SQInteger cmndLoadMotion(HSQUIRRELVM v);
SQInteger cmndLoadScript(HSQUIRRELVM v);
SQInteger cmndCreateChara(HSQUIRRELVM v);
SQInteger cmndDeleteChara(HSQUIRRELVM v);
SQInteger cmndMessageOpen(HSQUIRRELVM v);
SQInteger cmndRemoveParty(HSQUIRRELVM v);
SQInteger cmndAttachAvatar(HSQUIRRELVM v);
SQInteger cmndDeleteMotion(HSQUIRRELVM v);
SQInteger cmndGetSysIFlag(HSQUIRRELVM v);
SQInteger cmndSetSysIFlag(HSQUIRRELVM v);
SQInteger cmndAddTalkChara(HSQUIRRELVM v);
SQInteger cmndSetAntialiase(HSQUIRRELVM v);
SQInteger cmndSetShadowBone(HSQUIRRELVM v);
SQInteger cmndSetShadowFlag(HSQUIRRELVM v);
SQInteger cmndCharaHandle(HSQUIRRELVM v);
SQInteger cmndGetGameVersion(HSQUIRRELVM v);
SQInteger cmndGetPartyHandle(HSQUIRRELVM v);
SQInteger cmndResetTalkChara(HSQUIRRELVM v);
SQInteger cmndSetCharaGround(HSQUIRRELVM v);
SQInteger cmndSetShadowScale(HSQUIRRELVM v);
SQInteger cmndSyncEventChara(HSQUIRRELVM v);
SQInteger cmndGetElaspedFrame(HSQUIRRELVM v);
SQInteger cmndGetLanguageCode(HSQUIRRELVM v);
SQInteger cmndSetCharaCulling(HSQUIRRELVM v);
SQInteger cmndCreateGuestChara(HSQUIRRELVM v);
SQInteger cmndCreateManipChara(HSQUIRRELVM v);
SQInteger cmndDeleteGuestChara(HSQUIRRELVM v);
SQInteger cmndIsEquipmentManip(HSQUIRRELVM v);
SQInteger cmndBtlCharaHandle(HSQUIRRELVM v);
SQInteger cmndMessageReplaceVal(HSQUIRRELVM v);

#ifdef __cplusplus
}
#endif

#endif // IE4SQ_H