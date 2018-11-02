#ifndef _SIGN_H_
#define _SIGN_H_
#include <string>
using namespace std;

void InitSignCharset();
bool InCharSet(char ch);
bool InSignlist(char* ptr);
#endif