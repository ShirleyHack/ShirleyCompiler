#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_
#include "token.h"
#include "scanner.h"
struct proorder
{
    int value;
    int oriindex;
};
typedef struct proorder proorder;
void InitGrammar();
void PreProcessNon();
void ScanANonTer(Scanner *gr);
void ExtractLFactor(int startx, int endx);
void EliminateLRecur();
void ProcessPro();
void PrintPro();
void FreeMemory();
int FindIndex(char* cur);
int FindNonIndex(char* curnon);
int FindTerIndex(char* curter);
int IsNonTer(int index);
#endif