/*	see copyright notice in squirrel.h */
#ifndef _SQLEXER_H_
#define _SQLEXER_H_

#include "squirrel.h"
#include "sqobject.h"
#include "sqcompiler.h"

#ifdef SQUNICODE
typedef SQChar LexChar;
#else
typedef	unsigned char LexChar;
#endif

struct SQLexer
{
	SQLexer();
	~SQLexer();
	void Init(SQSharedState *ss,SQLEXREADFUNC rg,SQUserPointer up,CompilerErrorFunc efunc,void *ed);
	void Error(const SQChar *err);
	SQInteger Lex();
	const SQChar *Tok2Str(SQInteger tok);
private:
	SQInteger GetIDType(SQChar *s);
	SQInteger ReadString(SQInteger ndelim,bool verbatim);
	SQInteger ReadNumber();
	void LexBlockComment();
	SQInteger ReadID();
	void Next();
	SQInteger _curtoken;
	SQTable *_keywords;
public:
	SQInteger _prevtoken;
	SQInteger _currentline;
	SQInteger _lasttokenline;
	SQInteger _currentcolumn;
	const SQChar *_svalue;
	SQInteger _nvalue;
	SQFloat _fvalue;
	SQLEXREADFUNC _readf;
	SQUserPointer _up;
	LexChar _currdata;
	SQSharedState *_sharedstate;
	sqvector<SQChar> _longstr;
	CompilerErrorFunc _errfunc;
	void *_errtarget;
};

#endif
