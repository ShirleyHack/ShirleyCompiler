#ifndef _PARSER_H_
#define _PARSER_H_
#include "token.h"
#include "define.h"
struct Node
{
    char describe[30];
    Type type;
    char value[kTokenLen];
    int children[5];
};
typedef struct Node Node;
Node* CreateNewNode(Type type, char* value);
// typedef struct ChosenPro
// {
//     int line;
//     int next[10];
//     struct ChosenPro *pa;
// }ChosenPro;
// typedef struct ChosenPro ChosenPro;
ChosenPro* CreateNewPro(int line, ChosenPro *pa);
ChosenPro* CreateNewPro();
ChosenPro* FreePro(ChosenPro *target);
int PopNext(ChosenPro *target);
int NextPos(ChosenPro *target);
void Push(int* sta, int value);
int Pop(int* sta);
int Front(int* sta);
int IsEmpty(int* arr);
// void PrintUsage();
// void ProcessFile(int argc, char* argv[]);
void Parser(int argc, char* argv[]);
void BuildLLTable();
void PrintLLTable();
void BuildSets();
void BuildFirst();
void Print(int set[][kSetPerSize]);
void BuildFollow();
void PrintPlus();
// void PrintFollow()
void BuildFirstPlus();
int InSet(int fac, int set[]);
#endif