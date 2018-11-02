#ifndef _SCANNER_H_
#define _SCANNER_H_
#include <stdio.h>
#include <iostream>
#include <fstream>
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
            buffer[kLEnd] = EOF;
            buffer[kREnd] = EOF;
            fin.open(filename, ios::in);
        }
    Scanner(const char* filein, const char* fileout):forward(0), lread(false),
        rread(false), retracted(false), linenum(0),
        wordnum(0), charnum(0), state(Normal)
        {
            buffer[kLEnd] = EOF;
            buffer[kREnd] = EOF;
            fin.open(filein, ios::in);
            fout.open(fileout, ios::out);
        }
    // ~Scanner()
    // {
    //     token.clear();
    //     token.shrink_to_fit();
    // }
    ifstream fin;
    bool lread, rread, retracted;
    char curchar, buffer[kMaxBuf];
    int forward, linenum, wordnum, charnum;
    ofstream fout;
    void Tokenizer();
    void FillBuffer();
    void read();
    bool GetChar();
    void RefreshBuffer(int strptr);
    void retract();
    void SkipSpace();
    void SkipSpaceExceptLine();
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