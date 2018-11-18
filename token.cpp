#include "sign.h"
#include "token.h"
#include "externs.h"

Token* TokenInit()
{
    Token* newtoken = (Token*)malloc(sizeof(Token));
    newtoken->type = Token::None;
    newtoken->value = NULL;
    newtoken->next = NULL;
    return newtoken;
}

Token* TokenInitVal(Token::Type type, char* buf)
{
    Token* newtoken = (Token*)malloc(sizeof(Token));
    newtoken->type = Token::None;
    newtoken->next = NULL;
    char* ptr = buf;
    int i, size = 0;
    while(*ptr != '\0')
    {
        size++;
        ptr++;
    }
    size++;
    newtoken->value = (char*)malloc(sizeof(char)*size);
    for(i = 0; i < size; i++)
    {
        *(newtoken->value + i) = *(buf + i);
        // printf("%c ", *(value+i));
    }
    return newtoken;
}

TokenList* TokenListInit()
{
    TokenList* ptr = (TokenList*)malloc(sizeof(TokenList));
    ptr->head = (Token*)malloc(sizeof(Token));
    ptr->head = TokenInit();
    return ptr;
}

void TokenList::pushback(Token* newtoken)
{
    Token* cur = head;
    while(cur->next!=NULL)
        cur = cur->next;
    cur->next = newtoken;
    // printf("token: %s\n", newtoken->value);
}