#ifndef _TOKEN_H_
#define _TOKEN_H_
#include <string>
using namespace std;
class Token
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
    Token():type(None), value("\0") {}
    Token(Type type, char* buf):type(type)
    {
        char* ptr = buf;
        int size = sizeof(buf) / sizeof(buf[0]);
        value = (char*)malloc(sizeof(char)*size);
    }
};
bool InKeyword(string word);
#endif