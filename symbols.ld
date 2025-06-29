/* Inazuma Eleven Go Light & Shadow (EUR) Symbols */
__aeabi_memset = 0x1009cc;

/* aeabi */

/* string.h */
safeSprintf = 0x505D7C;
safeMemcpy = 0x506330;
safeMemcmp = 0x51d78c;
safestrcpy = 0x51d838;
safeStrlen = 0x51d8a8;
safeMemset = 0x51d92c;
safeStrcpy2 = 0x531744;
safeMemcpy2 = 0x534a20;
safeMemcpy3 = 0x544ea0;
safeStrncpy = 0x101254;
safeStrcmp = 0x4E90AC;

/* stdio.h */
safeMalloc = 0x514700;
safeMalloc2 = 0x52ab58;

/* assert.h */
_assert_func = 0x519028;

/* sqmem.h */
sq_vm_free = 0x124604;
sq_vm_malloc = 0x1265ec;
sq_vm_realloc = 0x12781c;
j_sq_vm_free = 0x4b44a8;
j_sq_vm_malloc = 0x4b4d38;
j_sq_vm_realloc = 0x4b63e8;

/* sqapi.h */
sq_newtable = 0x4B5238;
sq_aux_gettypedarg = 0x12d104;
sq_arraypop = 0x4B3600;
sq_bindenv = 0x4B3884;
sq_call = 0x4b3f08;
sq_getbool = 0x4B46E8;
sq_getclass = 0x4B473C;
sq_getfloat = 0x4B47F4;
sq_getinteger = 0x4b4930;
sq_getsize = 0x4B4A90;
sq_getstring = 0x4b4b9c;
sq_gettop = 0x4b4be4;
sq_gettype = 0x4B4BF4;
sq_move = 0x4B4D3C;
sq_newclass = 0x4B4E9C;
sq_newclass = 0x4b4e9c;
sq_newclosure = 0x4b4f90;
sq_newslot = 0x4b5118;
sq_newthread = 0x4B52D0;
sq_open = 0x4b5740;
sq_pop = 0x4b582c;
sq_push = 0x4b583c;
sq_pushbool = 0x4B587C;
sq_pushfloat = 0x4b58e0;
sq_pushinteger = 0x4b593c;
sq_pushnull = 0x4b5998;
sq_pushroottable = 0x4b5a0c;
sq_pushstring = 0x4b5a14;
sq_pushuserpointer = 0x4B5AB0;
sq_rawget = 0x4b5c44;
sq_rawset = 0x4B5F6C;
sq_rawset = 0x4b5f6c;
sq_readclosure = 0x4B6248;
sq_setnativeclosurename = 0x4B6A2C;
sq_setnativeclosurename = 0x4b6a2c;
sq_setparamscheck = 0x4B6B28;
sq_setparamscheck = 0x4b6b28;
sq_throwerror = 0x4b6fb0;
sq_tostring = 0x4B7038;
sq_wakeupvm = 0x4b70d8;

/* sql5.h */
sq_pushnull_L5 = 0x19efe4;
sq_pushtrue_L5 = 0x19EFB0;
sq_getinteger_cast_L5 = 0x22FD18;
sq_pushstring_L5 = 0x19F03C;
sq_gettypetag_L5 = 0x487778;
sq_getstring_L5 = 0x4878fc;
sq_countargs_L5 = 0x487a00;
sq_copy_stack_L5 = 0x19F0A0;

/* sqvm.h */
SQVM::Push = 0x168ff0;
SQVM::CallNative = 0x455a4c;
SQVM::PrintObjVal = 0x456a80;
SQVM::Raise_Error = 0x456bd4;
SQVM::CallMetaMethod = 0x456f04;
SQVM::Raise_IdxError = 0x457064;
SQVM::Raise_CompareError = 0x4571D8;
SQVM::Pop_0 = 0x45783c;
SQVM::Pop = 0x457880;
SQVM::Set = 0x4578a8;
SQVM::Top = 0x457d54;
SQVM::Call = 0x457d6c;
SQVM::Init = 0x457f58;
SQVM::Push2 = 0x458234;
SQVM::GetAt = 0x4586cc;
SQVM::GetUp = 0x4586d8;
SQVM::ObjCmp = 0x4586EC;
SQVM::Execute = 0x458d54;
SQVM::NewSlot = 0x45c334;
SQVM::CLASS_OP = 0x45CBC0;
SQVM::CLASS_OP = 0x45CBC0;
SQVM::ToString = 0x45D250;
SQVM::SQVM = 0x45db3c;

/* sqcompiler.h */
SQCompiler::Error = 0x100a64;
SQCompiler::Expression = 0x15a614;
SQCompiler::PrefixedExpr = 0x15b0a8;
SQCompiler::Statement = 0x15e2b8;

/* sqlexer.h */
SQLexer::ReadString = 0x464b5c;
SQLexer::Init = 0x46681C;

/* sqclass.h */
SQClass::SetAttributes = 0x463268;
SQClass::SQClass = 0x463d90;

/* sqtable.h */
SQTable::Get = 0x468998;
SQTable::SQTable = 0x469484;
SQTable::AllocNodes = 0x468838;

/* sqfuncstate.h */
SQFuncState::AddLineInfos = 0x1668f0;
SQFuncState::AddInstruction = 0x167438;

/* sqstate.h */
SQSharedState::Init = 0x19a9ec;
SQSharedState::SQSharedState = 0x19c774;
SQCollectable::AddToChain = 0x19a64c;
StringTable::Add = 0x169048;
CompileTypemask = 0x128FA0;

/* sqobject.h */
SQGenerator::Resume = 0x168898;
SQString::Create = 0x476364;
SQClosure::Load = 0x479B74;
GetTypeName = 0x124914;
IdType2Name = 0x12491c;

/* sqstdmath.h */
sqstd_register_mathlib = 0x4b7e6c;

/* sqstdstring.h */
sqstd_register_stringlib = 0x4b7f64;

/* sqcompiler.h */
Compile = 0x156E68;

/* sqbaselib.h */
thread_call = 0x124F58;
sq_base_register = 0x12b680;
_qsort_compare = 0x156900;

/* nn::fs */
nn__fs__Initialize = 0x4DAB60;
nn__fs__MountSaveData = 0x1D1300;
nn__fs__MountExtSaveData = 0x1D15D4;
nn__fs__ipc__FsFile__Write = 0x1D2F38;
nn__fs__ipc__FileSystem__OpenArchive = 0x538B54;
nn__fs__ipc__FileSystem__CloseArchive = 0x538BB4;
nn__fs__CTR__MPCore__detail__OpenExtSaveData = 0x1D21E8;
nn__fs__CTR__MPCore__detail__RegisterArchive = 0x4FB7AC;
nn__fs__CTR__MPCore__detail__OpenSpecialArchiveRaw = 0x1D2920;
nn__fs__CTR__MPCore__detail__FileServerArchive__File__TryWrite = 0x1D25CC;

/* nn::dsp */
nn__dsp__CTR__ClearSleepWakeUpCallback = 0x5257ec;

/* nn::cec */
nn__cec__CTR__detail__InitializeCecControl = 0x1D77F0;

/* nn::ndm */
nn__ndm__Initialize = 0x4E2828;

/* nn::cfg */
nn__cfg__CTR__detail__Initialize = 0x4FC220;

/* nn::err */
nn__err__CTR__ThrowFatalErrAll = 0x4FC5CC;

/* nn::srv */
nn__srv__Initialize = 0x4FCD20;
nn__srv__GetServiceHandle = 0x4FCE70;

/* VX2 */
VX2DecoderRelease = 0x12bce8;
VX2_Free = 0x1571e0;

/* crc32.h */
getCrc32 = 0x50619C;
crc32_table = 0x5737f4;

/* language */
getLanguage = 0x4e26bc;
getLanguageCode = 0x340718;

/* ie4sq.h */
/* IE4RegisterSquirrelFunc = 0x212B1C; */
createSquirrelVM = 0x1657b8;
getSquirrelVM = 0x497FD4;
register_global_func = 0x47cd48;
cmndRandom = 0x216d60;
cmndSetLv = 0x216E68;
cmndGetHash = 0x217230;
cmndGetHash = 0x217230;
cmndAddParty = 0x21759C;
cmndAddParty = 0x21759c;
cmndGetPosry = 0x2176D4;
cmndGetPosry = 0x2176d4;
cmndGpIsMax = 0x217820;
cmndKeyInput = 0x2178e0;
cmndSetPosry = 0x217FD8;
cmndSetPosry = 0x217fd8;
sysEventStep = 0x218490;
cmndLoadImage = 0x218A7C;
cmndLoadImage = 0x218a7c;
sysCacheClear = 0x21991c;
cmndLoadMotion = 0x219f64;
cmndLoadScript = 0x21a108;
cmndCreateChara = 0x21b598;
cmndDeleteChara = 0x21c190;
cmndMessageOpen = 0x21C504;
cmndMessageOpen = 0x21c504;
cmndRemoveParty = 0x21c890;
cmndAttachAvatar = 0x21ce98;
cmndDeleteMotion = 0x21d918;
cmndGetSysIFlag = 0x21da28;
cmndSetSysIFlag = 0x21eb70;
cmndAddTalkChara = 0x21f288;
cmndSetAntialiase = 0x220af4;
cmndSetShadowBone = 0x222fb4;
cmndSetShadowFlag = 0x2230b0;
cmndCharaHandle = 0x225278;
cmndGetGameVersion = 0x225730;
cmndGetPartyHandle = 0x2257C4;
cmndResetTalkChara = 0x2261d4;
cmndSetCharaGround = 0x2269f0;
cmndSetShadowScale = 0x226db0;
cmndSyncEventChara = 0x227154;
cmndGetElaspedFrame = 0x228984;
cmndGetLanguageCode = 0x228ADC;
cmndGetLanguageCode = 0x228adc;
cmndSetCharaCulling = 0x228f88;
cmndCreateGuestChara = 0x22d38c;
cmndCreateManipChara = 0x22d5e4;
cmndDeleteGuestChara = 0x22d6a0;
cmndIsEquipmentManip = 0x22dd7c;
cmndBtlCharaHandle = 0x22f650;
cmndMessageReplaceVal = 0x22FD18;

/* ie4btl.h */
BTL_RIVAL_PARTY_OFFSET = 0x5B0968;
BTL_PLAYER_OFFSET = 0x5B0B31;
getBtlPlayerOffset = 0x28DCC8;

/* ie4save.h */
saveLocationWrite = 0x16420C;

/* ie4party.h */
getPlayerOffset = 0x2fbb40;
canAddMorePartyMembers = 0x2FDF8C;

/* ie4io.h */
openSaveDataFile = 0x1641EC;
writeSaveDataFile = 0x16420C;
SAVE_DATA_OFFSET = 0x5C4EE8;
initSaveFileHandle = 0x52B9E0;
closeSaveFileHandle = 0x53CFA8;

/* ie4std.h */
allocatorAlloc = 0x51F98C;
allocatorFree = 0x507E48;

/* crc32.h */
getCrc32 = 0x50619C;

/* main.c */
main = 0x100000;

/* unknown source */
FASTAUDIO_DECODE = 0x10f2f4;
mwmomobiclipDecoderRelease = 0x4b2738;
MountRom = 0x4DADA8;