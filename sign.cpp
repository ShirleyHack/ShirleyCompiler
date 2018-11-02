#include <string>
#include "sign.h"
#include "externs.h"
using namespace std;
void InitSignCharset()
{
    int i;
    for(i = 0; signlist[i][0] != '\0'; i++)
    {
        int length = signlist[i].length();
        for(int j = 0; j<length; j++)
            signcharset.insert(signlist[i][j]);
    }
}
bool InCharSet(char ch)
{
    if(signcharset.find(ch) == signcharset.end())
        return false;
    return true;
}
bool InSignlist(char* ptr)
{
    int i;
    string str = ptr;
    for(i = 0; signlist[i][0] != '\0'; i++)
    {
        if(signlist[i].compare(str) == 0)
            return true;
    }
    return false;
}
