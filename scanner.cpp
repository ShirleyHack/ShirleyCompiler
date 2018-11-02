#include <cctype>
#include <unordered_set>
#include "sign.h"
#include "token.h"
#include "externs.h"
#include "scanner.h"

char file[kFileName], err[kFileName];
char readbuf[kStrBuf];
bool EndOfFile;
std::list<const Token*> tokenlist;
unordered_set<char> signcharset;
const string signlist[] =
{
    "+", "-", "*", "/", "%",
    ">", "<", ">=", "<=", "=", "!=", "==", "=~",
    "+=", "-=", "*=", "/=", "%=",
    "&&", "||", "!", "^",
    "&&=", "||=", "^=",
    "<<", ">>", "->", "<-",
    "?", ":",
    ".", ",", ";", "..",
    "(", ")", "[", "]", "{", "}", "|",
    "@", "@@", "$", "\0"
};
const string keyword[] =
{
	"auto",    "break",   "case",    "char",
    "const",   "continue","default", "do",
    "double",  "else",    "enum",    "extern",
    "float",   "for",     "goto",    "if",
    "int",     "long",    "register","return",
    "short",   "signed",  "static",  "sizeof",
    "struct",  "switch",  "typedef", "union",
    "unsigned","void",    "volatile","while",
    "\0"
};

void Scanner::FillBuffer()
{
    if(!lread)
    {
        rread = false;
        lread = true;
        fin.read(buffer, kLEnd);
    }
    else if(lread && !rread)
    {
        lread = false;
        rread = true;
        fin.read(buffer+kLEnd+1, kLEnd);
    }
    else
        fprintf(stderr, "Failed to fill buffer");
    if(fin.gcount() != kLEnd)
    {
        buffer[fin.gcount()] = EOF;
        EndOfFile = true;
    }
    EndOfFile = false;
}
// 需要完善：return false能不能好看些
bool Scanner::GetChar()
{
    curchar = buffer[forward];
    if(curchar == '\n')
        linenum++;
    else if(isalnum(curchar))
        charnum++;
    if(buffer[++forward] == EOF)
    {
        if(EndOfFile)
            return false;
        if(!retracted)
            FillBuffer();
        if(forward == kLEnd)
            forward++;
        else if(forward == kREnd)
            forward = kStart;
    }
    if(retracted)
        retracted = false;
    return true;
}
void Scanner::SkipSpace()
{
    while(curchar == ' ' || curchar == '\n' ||
        curchar == '\t')
        GetChar();
}

void Scanner::read()
{
    int strptr = 0;
    bool haspoint = false;
    char formerchar = '\0';
    Token::Type tokentype = Token::None;
    while(GetChar())
    {
        if(state == Scanner::Normal)
        {
            if(curchar == '#')
            {
                readbuf[strptr++] = curchar;
                state = Scanner::Macro;
            }
            else if(IsIdentifierFirst(curchar))
            {
                readbuf[strptr++] = curchar;
                state = Scanner::Identifier;
            }
            else if(curchar == '/')
            {
                if(TryNext('/'))
                {
                    formerchar = '/';
                    state = Scanner::Annotation;
                }
                else if(TryNext('*'))
                {
                    state = Scanner::Annotation;
                    formerchar = '*';
                }
                else
                    fprintf(stderr, "Unexpected char '/'.\n");
            }
            else if(InCharSet(curchar))
            {
                readbuf[strptr++] = curchar;
                if((curchar == '-' || curchar == '+') && TryNext('d'))
                    state = Scanner::Number;
                else
                    state = Scanner::Sign;
            }
            else if(curchar == '\"' || curchar == '\'')
            {
                readbuf[strptr++] = curchar;
                formerchar = curchar;
                state = Scanner::String;
            }
            else if(curchar == ' ' || curchar == '\t' ||
                curchar == '\n')
            {
                SkipSpace();
            }
            // else if()
            // {
            //     tokentype = Token::NewLine;
            // }
            else if(curchar == EOF)
            {
                readbuf[strptr++] = curchar;
                tokentype = Token::EndSymbol;
            }
            else
                fprintf(stderr, "Failed at read().\n");
        }
        else if(state == Scanner::Macro)
        {
            readbuf[strptr++] = curchar;
            if(curchar == '\n')
            {
                tokentype = Token::Macro;
                state = Scanner::Normal;
            }
        }
        else if(state == Scanner::Identifier)
        {
            if(InIdentifierNotSpecific(curchar))
                readbuf[strptr++] = curchar;
            else
            {
                if(InKeyword(readbuf))
                    tokentype = Token::Keyword;
                else
                    tokentype = Token::Identifier;
                state = Scanner::Normal;
                retract();
            }
        }
        else if(state == Scanner::Annotation)
        {
            if((formerchar == '/' && curchar != '\n') ||
                (formerchar == '*' && curchar != '*'))
                readbuf[strptr++] = curchar;
            else if(formerchar == '*' && curchar == '*')
            {
                if(TryNext('/'))
                {
                    tokentype = Token::Sign;
                    state = Scanner::Normal;
                }
                else
                    readbuf[strptr++] = curchar;
            }
            else if(formerchar == '/' && curchar == '\n')
            {
                tokentype = Token::Sign;
                state = Scanner::Normal;
            }
            else if(formerchar == '/' && TryNext(EOF))
            {
                tokentype = Token::Sign;
                state = Scanner::Normal;
            }
            else
                fprintf(stderr, "Wrong annotation format.\n");
        }
        else if(state == Scanner::Number)
        {
            if(isdigit(curchar))
                readbuf[strptr++] = curchar;
            else if(curchar == '.' && haspoint == false)
            {
                readbuf[strptr++] = curchar;
                haspoint = true;
            }
            else
            {
                if(haspoint)
                    tokentype = Token::Float;
                else
                    tokentype = Token::Int;
                state = Scanner::Normal;
            }
        }
        else if(state == Scanner::Sign)
        {
            if(InCharSet(curchar))
                readbuf[strptr++] = curchar;
            else
            {
                // string str = readbuf;
                if(InSignlist(readbuf))
                {
                    tokentype = Token::Sign;
                    state = Scanner::Normal;
                    retract();
                }
                else
                    fprintf(stderr, "Unexpected sign \"%s\"!\n", readbuf);
            }
        }
        //here

        else if(state == Scanner::String)
        {
            if(TryNext(EOF))
                fprintf(stderr, "Wrong string format. Expecting a \'%c\'\n", formerchar);
            if(formerchar != curchar)
                readbuf[strptr++] = curchar;
            else
            {
                tokentype = Token::String;
                state = Scanner::Normal;
            }
        }
        // else if(state == Scanner::RegEx)
        // {
        //     /*wait for supplement.*/
        // }
        // else if(state == Scanner::Space)
        // {

        // }
        if(tokentype != Token::None)
        {
            printf("[1]sizeof tokenlist: %d\n", tokenlist.size());
            if(tokentype == Token::Annotation)
                fout << readbuf << endl;
            else
                tokenlist.push_back(new Token(tokentype, readbuf));
                printf("[2]sizeof tokenlist: %d\n", tokenlist.size());
            tokentype = Token::None;
            formerchar = '\0';
            haspoint = false;
            strptr = 0;
        }
    }
}

void Scanner::Tokenizer()
{
    Initialize();
    FillBuffer();
    while(!(EndOfFile && buffer[forward]))
    {
        SkipSpace();
        read();
    }
    PrintTokenList();
}

void Scanner::retract()
{
    if(forward == 0)
        forward = kREnd - 1;
    else
    {
        forward--;
        if(forward == kLEnd)
            forward--;
    }
    retracted = true;
}

bool Scanner::TryNext(char ch)
{
    if(GetChar())
    {
        if(ch == 'd' && isdigit(curchar))
            return true;
        else
        {
            if(curchar == ch)
                return true;
            else retract();
        }
    }
    return false;
}
// void Scanner::RefreshBuffer(int strptr)
// {
//     readbuf[strptr++] = curchar;
// }
void Initialize()
{
    EndOfFile = false;
    InitSignCharset();
}

char* GetExtension(char* filename)
{
    char delimiter = '.';
    char *fileptr = filename;
    int i, mark = 0;
    for(i = 0; *(fileptr+i) != '\0'; i++)
        if(*(fileptr+i) == delimiter)
            mark = i;
    if(mark != 0 && *(fileptr+mark) == delimiter)
        return fileptr + mark + 1;
    return NULL;
}

bool ProcessInput()
{
    printf("Please type your C file name and errlog name(for example: a.cpp/a.c, err.txt):\n");
    scanf("%s%s", file, err);
    char *extension = GetExtension(file);
    char *errext = GetExtension(err);
    printf("fileext:%s, errext:%s\n", extension, errext);
    if(((extension[0] == 'c' && extension[1] == '\0') ||
        (extension[0] == 'c' && extension[1] == 'p' &&
        extension[2] == 'p' && extension[3] == '\0')) &&
        errext[0] == 't' && errext[1] == 'x' &&
        errext[2] == 't' && errext[3] == '\0')
        return true;
    return false;
}

bool InIdentifierNotSpecific(char ch)
{
    return isalnum(ch) || ch == '_';
}
bool IsIdentifierFirst(char ch)
{
    return isalpha(ch) || ch == '_';
}
void PrintTokenList()
{
    std::list<const Token*>::iterator it;
    for(it = tokenlist.begin(); it != tokenlist.end(); it++)
    {
        const Token* tmp = *it;
        printf("Type: %-18s Value: %s\n",
            tmp->type == Token::Macro ? "Macro" :
            tmp->type == Token::Keyword ? "Keyword" :
            tmp->type == Token::Int ? "Int" :
            tmp->type == Token::Float ? "Float" :
            tmp->type == Token::Identifier ? "Identifier" :
            tmp->type == Token::Annotation ? "Annotation" :
            tmp->type == Token::Sign ? "Sign" :
            tmp->type == Token::String ? "String" :
            tmp->type == Token::EndSymbol ? "EndSymbol" :
            "Wrong syntax!", tmp->value);
    }
}
// void Destruct()
// {
//     KeyWord.clear();
//     KeyWord.shrink_to_fit();
// }
int main()
{
    if(!ProcessInput())
    {
        fprintf(stderr, "Wrong format.\n");
        return -1;
    }
    Scanner sc = Scanner(file, err);
    sc.Tokenizer();
}