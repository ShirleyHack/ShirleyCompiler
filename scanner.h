#ifndef _SCANNER_H_
#define _SCANNER_H_
#include <stdio.h>
#include <string.h>
#include <list>
#include "sign.h"
#include "externs.h"
#include "token.h"
using namespace std;

class Scanner
{
public:
    enum State
    {
        Normal, Sign, Annotation, String, Identifier,
        Number, Macro
        // RegEx, Space,
    };
    Scanner():forward(0), lread(false), rread(false),
        retracted(false), linenum(0), wordnum(0),
        charnum(0), state(Normal)
        {
            buffer[kLEnd] = EOF;
            buffer[kREnd] = EOF;
        }
    Scanner(const char* filename):forward(0), lread(false),
        rread(false), retracted(false), linenum(0),
        wordnum(0), charnum(0), state(Normal)
        {
            memset(buffer, 0 ,sizeof(buffer));
            buffer[kLEnd] = EOF;
            buffer[kREnd] = EOF;
            fin = fopen(filename, "r");
        }
    Scanner(const char* filein, const char* fileerr, const char* filedoc):forward(0), lread(false),
        rread(false), retracted(false), linenum(0),
        wordnum(0), charnum(0), state(Normal)
        {
            memset(buffer, 0 ,sizeof(buffer));
            buffer[kLEnd] = EOF;
            buffer[kREnd] = EOF;
            fin = fopen(filein, "r");
            ferr = fopen(fileerr, "w");
            fdoc = fopen(filedoc, "w");
        }
    // ~Scanner()
    // {
    //     token.clear();
    //     token.shrink_to_fit();
    // }
    FILE *fin, *ferr, *fdoc;
    bool lread, rread, retracted;
    char curchar, buffer[kMaxBuf];
    int forward, linenum, wordnum, charnum;
    void Tokenizer();
    void FillBuffer();
    void read();
    bool GetChar();
    // void RefreshBuffer(int strptr);
    void retract();
    // void SkipSpace();
    // void SkipSpaceExceptLine();
    bool TryNext(char ch);
private:
    State state;
};
void Initialize();
bool IsIdentifierFirst(char ch);
bool InIdentifierNotSpecific(char ch);
void init();
void PrintTokenList();
#endif