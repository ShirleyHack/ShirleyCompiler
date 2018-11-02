#include "sign.h"
#include "token.h"
#include "externs.h"
bool InKeyword(string word)
{
    int i;
    for(i = 0; keyword[i][0] != '\0'; i++)
    {
        if(keyword[i].compare(word))
            return true;
    }
    return false;
}