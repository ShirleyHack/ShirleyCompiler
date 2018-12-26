#include "sign.h"
#include "token.h"
#include "externs.h"

Token* CreateNewToken()
{
    Token* newtoken = (Token*)malloc(sizeof(Token));
    newtoken->type = Type::None;
    memset(newtoken->value, 0, sizeof(newtoken->value));
    newtoken->next = NULL;
    return newtoken;
}

Token* CreateNewToken(Type type, char* value)
{
    Token* newtoken = (Token*)malloc(sizeof(Token));
    newtoken->type = type;
    newtoken->next = NULL;
    strcpy(newtoken->value, value);
    return newtoken;
}

TokenList* TokenListInit()
{
    TokenList* ptr = (TokenList*)malloc(sizeof(TokenList));
    ptr->head = CreateNewToken();
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
void PrintToken(Token* word)
{
    printf("Type: %-18s Value: %s\n",
        word->type == Type::Macro ? "Macro" :
        word->type == Type::Keyword ? "Keyword" :
        word->type == Type::Int ? "Int" :
        word->type == Type::Float ? "Float" :
        word->type == Type::Identifier ? "Identifier" :
        word->type == Type::Annotation ? "Annotation" :
        word->type == Type::Operator ? "Sign" :
        word->type == Type::String ? "String" :
        word->type == Type::EndSymbol ? "EndSymbol" :
        "Wrong syntax!", word->value);
}