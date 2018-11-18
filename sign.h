#ifndef _SIGN_H_
#define _SIGN_H_
#include <string>
#include <string.h>
using namespace std;

void InitSignCharset(const char* signlist[], int signlistsize);
bool InCharSet(char ch);
bool InStrList(const char* strlist[], char* ptr, int strlistsize);
bool InStrList(const char* strlist[], char* ptr, int len, int strlistsize);
#endif