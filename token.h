#ifndef _TOKEN_H_
#define _TOKEN_H_

struct Token
{
public:
    enum Type
    {
        None = 0 , Keyword, Number, Identifier, Sign,
        Annotation, String, EndSymbol, Macro, Int, Float
        // RegEx, Space, NewLine
    };
    Type type;
    char *value;
    Token *next;
};
Token* TokenInit();
Token* TokenInitVal(Token::Type type, char* buf);

struct TokenList
{
public:
    Token* head;
    void pushback(Token* newtoken);
};
TokenList* TokenListInit();
#endif