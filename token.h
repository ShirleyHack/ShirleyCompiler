#ifndef _TOKEN_H_
#define _TOKEN_H_
#include "define.h"
/*
struct NormalNode
{
    char content[30];//内容
    char describe[30];//描述
    int type;//种别码
    int addr;//入口地址
    int line;//所在行数
    NormalNode * next;//下一个节点
};
*/

struct Token
{
public:

    Type type;
    char value[kTokenLen];
    Token *next;
};
typedef struct Token Token;
void PrintToken(Token* word);
Token* CreateNewToken();
Token* CreateNewToken(Type type, char* buf);

struct TokenList
{
public:
    Token* head;
    void pushback(Token* newtoken);
};
TokenList* TokenListInit();
#endif