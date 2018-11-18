#ifndef _EXTERNS_H_
#define _EXTERNS_H_
#include <unordered_set>
#include <list>
#include "token.h"
#include "sign.h"
#define kStrBuf 500
#define kFileName 105
#define kMaxBuf 85
#define kLEnd 40
#define kMiddle 41
#define kREnd 81
#define kStart 0
// extern std::list<const Token*> tokenlist;
extern TokenList* tokenlist;
extern unordered_set<char> signcharset;
extern const char* signlist[];
extern const char* keyword[];
extern int signlistsize, keylistsize, markend, wrongnum;
extern char readbuf[kStrBuf];
#endif