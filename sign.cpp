#include <string>
#include <string.h>
#include "sign.h"
#include "externs.h"
using namespace std;
void InitSignCharset(const char* signlist[], int signlistsize)
{
    int i;
    for(i = 0; i < signlistsize; i++)
    {
        for(int j = 0; signlist[i][j] != '\0'; j++)
            signcharset.insert(signlist[i][j]);
    }
}
bool InCharSet(char ch)
{
    if(signcharset.find(ch) == signcharset.end())
        return false;
    return true;
}

bool InStrList(const char* strlist[], char* ptr, int strlistsize)
{
    int i, j;
    for(i = 0; i < strlistsize; i++)
    {
        if(strcmp(ptr, strlist[i]) == 0)
            return true;
    }
    return false;
}
bool InStrList(const char* strlist[], char* ptr, int len, int strlistsize)
{
    int i, j;
    for(i = 0; i < strlistsize; i++)
    {
        if(strncmp(ptr, strlist[i], len) == 0)
            return true;
    }
    return false;
}