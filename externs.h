#ifndef _EXTERNS_H_
#define _EXTERNS_H_
#include <unordered_set>
#include <list>
#include "define.h"
#include "sign.h"
#include "token.h"

// extern std::list<const Token*> tokenlist;
extern TokenList* tokenlist;
extern unordered_set<char> signcharset;
extern const char* signlist[];
extern const char* keyword[];
extern int signlistsize, keylistsize;
// markend, wrongnum, forwardptr;
extern int ntersize, extraend, tablex, ioft[kProSize], table[kProSize][kProLen];
// extern bool EndOfFile, lread, rread, retracted;
// extern char curchar, readbuf[kMaxBuf];
extern char empsym[], stasym[], eofsym[];
extern char* factor[kTerSize + kProSize];
#endif