#ifndef _SCANNER_H_
#define _SCANNER_H_
#include <stdio.h>
#include <string.h>
#include <list>
#include "sign.h"
#include "externs.h"
#include "token.h"
using namespace std;


struct Scanner
{
// public:
    // enum State
    // {
    //     Normal, Sign, Annotation, String, Identifier,
    //     Number, Macro
    //     // RegEx, Space,
    // };
    // Scanner(): linenum(0), wordnum(0),
    //     charnum(0), state(Normal)
    //     {
    //         buffer[kLEnd] = EOF;
    //         buffer[kREnd] = EOF;
    //     }
    // Scanner(const char* filename): linenum(0),
    //     wordnum(0), charnum(0), state(Normal)
    //     {
    //         memset(buffer, 0 ,sizeof(buffer));
    //         buffer[kLEnd] = EOF;
    //         buffer[kREnd] = EOF;
    //         fin = fopen(filename, "r");
    //     }
    // Scanner(const char* filein, const char* fileerr, const char* filedoc): linenum(0),
    //     wordnum(0), charnum(0), state(Normal)
    //     {
    //         memset(buffer, 0 ,sizeof(buffer));
    //         buffer[kLEnd] = EOF;
    //         buffer[kREnd] = EOF;
    //         fin = fopen(filein, "r");
    //         ferr = fopen(fileerr, "w");
    //         fdoc = fopen(filedoc, "w");
    //     }
    // ~Scanner()
    // {
    //     token.clear();
    //     token.shrink_to_fit();
    // }
    FILE *fin, *ferr, *fdoc;
    int linenum, wordnum, charnum, markend, wrongnum, forwardptr;
    int EndOfFile, lread, rread, retracted;
    char curchar, buffer[kMaxBuf];

    // void FillBuffer();
    // Token* NextWord();
    // bool GetChar();
    // void RefreshBuffer(int strptr);
    // void retract();
    // // void SkipSpace();
    // // void SkipSpaceExceptLine();
    // bool TryNext(char ch);
    State state;
};
typedef struct Scanner Scanner;

void FillBuffer(Scanner* sc);
Token* NextWord(Scanner* sc);
int GetChar(Scanner *sc);
void retract(Scanner *sc);
int TryNext(Scanner *sc, char ch);
Scanner* CreateNewScanner();
Scanner* CreateNewScanner(const char* filename);
Scanner* CreateNewScanner(const char* filein, const char* fileerr, const char* filedoc);
Scanner* InitLexAnaly(int argc, char* argv[]);
int IsIdentifierFirst(char ch);
int InIdentifierNotSpecific(char ch);
int SizeOfArr(int* arr);
void init();
void PrintTokenList();
#endif