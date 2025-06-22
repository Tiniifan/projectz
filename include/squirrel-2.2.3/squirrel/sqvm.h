/*
	see copyright notice in squirrel.h
*/
#ifndef _SQVM_H_
#define _SQVM_H_

#include <new>
#include "sqobject.h" 
#include "sqtable.h"
#include "sqopcodes.h"

#define MAX_CALLS 100

// On GCC/clang, we can use this to get the address of the label
#ifdef __GNUC__
#define SQ_GNUC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if SQ_GNUC_VERSION >= 30400
#define USE_COMPUTED_GOTOS
#endif
#endif

#ifdef USE_COMPUTED_GOTOS
#define INIT_COMPUTED_GOTOS() \
	static const void* _g_instr_jumps[] = { \
		&&_OP_LINE_TAG, &&_OP_LOAD_TAG, &&_OP_LOADINT_TAG, &&_OP_LOADFLOAT_TAG, &&_OP_DLOAD_TAG, \
		&&_OP_TAILCALL_TAG, &&_OP_CALL_TAG, &&_OP_PREPCALL_TAG, &&_OP_PREPCALLK_TAG, &&_OP_GETK_TAG, \
		&&_OP_MOVE_TAG, &&_OP_NEWSLOT_TAG, &&_OP_DELETE_TAG, &&_OP_SET_TAG, &&_OP_GET_TAG, \
		&&_OP_EQ_TAG, &&_OP_NE_TAG, &&_OP_ARITH_TAG, &&_OP_BITW_TAG, &&_OP_RETURN_TAG, \
		&&_OP_LOADNULLS_TAG, &&_OP_LOADROOTTABLE_TAG, &&_OP_LOADBOOL_TAG, &&_OP_DMOVE_TAG, &&_OP_JMP_TAG, \
		&&_OP_JNZ_TAG, &&_OP_JZ_TAG, &&_OP_LOADFREEVAR_TAG, &&_OP_VARGC_TAG, &&_OP_GETVARGV_TAG, \
		&&_OP_NEWTABLE_TAG, &&_OP_NEWARRAY_TAG, &&_OP_APPENDARRAY_TAG, &&_OP_GETPARENT_TAG, &&_OP_COMPARITH_TAG, \
		&&_OP_COMPARITHL_TAG, &&_OP_INC_TAG, &&_OP_INCL_TAG, &&_OP_PINC_TAG, &&_OP_PINCL_TAG, \
		&&_OP_CMP_TAG, &&_OP_EXISTS_TAG, &&_OP_INSTANCEOF_TAG, &&_OP_AND_TAG, &&_OP_OR_TAG, \
		&&_OP_NEG_TAG, &&_OP_NOT_TAG, &&_OP_BWNOT_TAG, &&_OP_CLOSURE_TAG, &&_OP_YIELD_TAG, \
		&&_OP_RESUME_TAG, &&_OP_FOREACH_TAG, &&_OP_POSTFOREACH_TAG, &&_OP_DELEGATE_TAG, &&_OP_CLONE_TAG, \
		&&_OP_TYPEOF_TAG, &&_OP_PUSHTRAP_TAG, &&_OP_POPTRAP_TAG, &&_OP_THROW_TAG, &&_OP_CLASS_TAG, \
		&&_OP_NEWSLOTA_TAG \
	};
#define COMPUTED_GOTO(op) { goto *_g_instr_jumps[op]; }
#define SQUIRREL_VM_SWITCH_GOTO() COMPUTED_GOTO(_i_.op)
#else
#define SQUIRREL_VM_SWITCH_GOTO() switch(_i_.op)
#endif

struct SQExceptionTrap{
	SQExceptionTrap() {}
	SQExceptionTrap(SQInteger ss, SQInteger sb,const SQInstruction *ip, SQInteger et)
	{	_stacksize = ss; _stackbase = sb; _ip = ip; _extarget = et; }
	SQExceptionTrap(const SQExceptionTrap &et)
	{	_stacksize = et._stacksize; _stackbase = et._stackbase; _ip = et._ip; _extarget = et._extarget; }
	SQInteger _stacksize;
	SQInteger _stackbase;
	const SQInstruction *_ip;
	SQInteger _extarget;
};

#define _INLINE 

typedef sqvector<SQExceptionTrap> ExceptionsTraps;

enum ExecutionType {
	ET_CALL,
	ET_RESUME_GENERATOR,
	ET_RESUME_VM
};

struct SQVM : CHAINABLE_OBJ
{
	struct CallInfo{
		const SQInstruction *_ip;
		SQObjectPtr *_literals;
		SQObjectPtr _closure;
		SQInteger _etraps;
		SQInteger _prevtop;
		SQInteger _prevstkbase;
		SQInt32  _target;
		SQBool _root;
		SQInt16 _ncalls;
		struct VarArgs{
			SQInt32 _base;
			SQInt32 _size;
		} _vargs;
		SQGenerator *_generator;
	};

	typedef sqvector<CallInfo> CallInfoVec;
	
public:
	SQVM(SQSharedState *ss);
	~SQVM();
	
	bool Init(SQVM *friendvm, SQInteger stacksize);
	bool Call(SQObjectPtr &closure,SQInteger nparams,SQInteger stackbase,SQObjectPtr &outres,SQBool raiseerror);
	bool Execute(SQObjectPtr &closure, SQInteger target, SQInteger nargs, SQInteger stackbase,SQObjectPtr &outres, SQBool raiseerror,ExecutionType et);
	
	void Push(const SQObjectPtr &o);
	void Pop(SQInteger n);
	SQObjectPtr &Top();
	SQObjectPtr &GetUp(SQInteger n);
	SQObjectPtr &GetAt(SQInteger n);

	void Raise_Error(const SQChar *s, ...);
	void Raise_IdxError(const SQObjectPtr &o);
	void Raise_CompareError(const SQObjectPtr &o1, const SQObjectPtr &o2);

	bool Set(const SQObjectPtr &self,const SQObjectPtr &key,const SQObjectPtr &val,bool fetchroot);
	bool NewSlot(const SQObjectPtr &self,const SQObjectPtr &key,const SQObjectPtr &val,bool bstatic);
	bool ObjCmp(const SQObjectPtr &o1,const SQObjectPtr &o2,SQInteger &result);
	void ToString(const SQObjectPtr &o,SQObjectPtr &res);
	bool CallMetaMethod(SQDelegable *del,SQMetaMethod mm,SQInteger nparams,SQObjectPtr &outres);
	SQRESULT Suspend();

	// public members
	SQObjectPtrVec _stack;
	SQInteger _top;
	SQInteger _stackbase;
	SQObjectPtr _roottable;
	SQObjectPtr _lasterror;
	SQObjectPtr _errorhandler;
	SQObjectPtr _debughook;
	SQUserPointer _foreignptr;
	SQObjectPtr temp_reg;
	
	CallInfo* _callsstack;
	SQInteger _callsstacksize;
	SQInteger _alloccallsstacksize;
	CallInfoVec _callstackdata;

	ExceptionsTraps _etraps;
	CallInfo* ci;
	SQObjectPtrVec _vargsstack;
	
	SQBool _suspended;
	SQBool _suspended_root;
	SQInteger _suspended_target;
	SQInteger _suspended_traps;
	CallInfo::VarArgs _suspend_varargs;
	
	SQInteger _nnativecalls;
	SQSharedState *_sharedstate;

private:
	void Finalize();
	bool BW_OP(SQUnsignedInteger op,SQObjectPtr &trg,const SQObjectPtr &o1,const SQObjectPtr &o2);
	bool ARITH_OP(SQUnsignedInteger op,SQObjectPtr &trg,const SQObjectPtr &o1,const SQObjectPtr &o2);
	bool NEG_OP(SQObjectPtr &trg,const SQObjectPtr &o);
	bool CMP_OP(CmpOP op, const SQObjectPtr &o1,const SQObjectPtr &o2,SQObjectPtr &res);
	bool StringCat(const SQObjectPtr &str,const SQObjectPtr &obj,SQObjectPtr &dest);
	void TypeOf(const SQObjectPtr &obj1,SQObjectPtr &dest);
	bool StartCall(SQClosure *closure,SQInteger target,SQInteger args,SQInteger stackbase,bool tailcall);
	bool Return(SQInteger _arg0, SQInteger _arg1, SQObjectPtr &retval);
	bool IsFalse(SQObjectPtr &o);
	void CallErrorHandler(SQObjectPtr &error);
	void CallDebugHook(SQInteger type,SQInteger forcedline=0);
	bool Get(const SQObjectPtr &self,const SQObjectPtr &key,SQObjectPtr &dest,bool raw, bool fetchroot);
	bool Clone(const SQObjectPtr &self,SQObjectPtr &target);
	bool DeleteSlot(const SQObjectPtr &self,const SQObjectPtr &key,SQObjectPtr &res);
	bool CreateClassInstance(SQClass *theclass, SQObjectPtr &inst, SQObjectPtr &constructor);
	void PopVarArgs(CallInfo::VarArgs &vargs);
	bool FOREACH_OP(SQObjectPtr &o1,SQObjectPtr &o2,SQObjectPtr &o3,SQObjectPtr &o4,SQInteger arg_2,int exitpos,int &jump);
	bool DELEGATE_OP(SQObjectPtr &trg,SQObjectPtr &o1,SQObjectPtr &o2);
	bool CLOSURE_OP(SQObjectPtr &target, SQFunctionProto *func);
	bool GETVARGV_OP(SQObjectPtr &target,SQObjectPtr &index,CallInfo *ci);
	bool GETPARENT_OP(SQObjectPtr &o,SQObjectPtr &target);
	bool LOCAL_INC(SQInteger op,SQObjectPtr &target, SQObjectPtr &a, SQObjectPtr &incr);
	bool PLOCAL_INC(SQInteger op,SQObjectPtr &target, SQObjectPtr &a, SQObjectPtr &incr);
	bool DerefInc(SQInteger op,SQObjectPtr &target, SQObjectPtr &self, SQObjectPtr &key, SQObjectPtr &incr, bool postfix);
	void Remove(SQInteger n);
	void ClearStack(SQInteger last_top);
	bool CallNative(SQNativeClosure *nclosure,SQInteger nargs,SQInteger stackbase,SQObjectPtr &retval,bool &suspend);
};

#define Raise_Error(v,desc,...) v->Raise_Error(desc,__VA_ARGS__)

#define STK(n) _stack._vals[(_stackbase-1)+n]
#define TARGET _stack._vals[_stackbase-1+arg0]

#define PUSH_CALLINFO(v,ci) { \
	if(v->_callsstacksize >= v->_alloccallsstacksize) { \
		v->_callstackdata.resize(v->_alloccallsstacksize*2); \
		v->_alloccallsstacksize = v->_callstackdata.size(); \
		v->_callsstack = &v->_callstackdata[0]; \
	} \
	v->_callsstack[v->_callsstacksize++] = ci; \
	v->ci = &v->_callsstack[v->_callsstacksize-1]; \
}

#define POP_CALLINFO(v) { \
	v->_callsstacksize--; \
	v->ci = v->_callsstacksize?&v->_callsstack[v->_callsstacksize-1]:NULL; \
}

#endif //_SQVM_H_