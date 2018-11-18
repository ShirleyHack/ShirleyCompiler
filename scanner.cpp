#include <cctype>
#include <unordered_set>
#include "sign.h"
#include "token.h"
#include "externs.h"
#include "scanner.h"

char *file, *err, *doc;
char readbuf[kStrBuf];
int signlistsize, keylistsize, markend, wrongnum;
bool EndOfFile;
TokenList* tokenlist;
unordered_set<char> signcharset;
const char* signlist[]=
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
    "$", "\0"
};
const char* keyword[]=
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
    int i;
    if(!lread)
    {
        rread = false;
        lread = true;
        for(i = 0; i < kLEnd; i++)
        {
            if((buffer[i] = fgetc(fin)) == EOF)
            {
                EndOfFile = true;
                markend = i + 1;
                break;
            }            
        }
    }
    else if(lread && !rread)
    {
        lread = false;
        rread = true;
        for(i = kLEnd + 1; i < kLEnd + kLEnd + 1; i++)
        {
            if((buffer[i] = fgetc(fin)) == EOF)
            {
                EndOfFile = true;
                markend = i + 1;
                break;
            }            
        }
    }
    else
        fprintf(ferr, "[%d]: Failed to fill buffer", ++wrongnum);
}
// 需要完善：return false能不能好看些
bool Scanner::GetChar()
{
    if(forward == markend && EndOfFile)
        return false;
    curchar = buffer[forward];
    if(curchar == '\n')
        linenum++;
    else if(isalnum(curchar))
        charnum++;
    if(buffer[++forward] == EOF)
    {
        if(!retracted && !EndOfFile)
            FillBuffer();
        if(forward == markend && EndOfFile) {}
        else if(forward == kLEnd)
            forward++;
        else if(forward == kREnd)
            forward = kStart;
    }
    if(retracted)
        retracted = false;
    return true;
}
// void Scanner::SkipSpace()
// {
//     while(curchar == ' ' || curchar == '\n' ||
//         curchar == '\t')
//         GetChar();
// }

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
                readbuf[strptr++] = curchar;
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
                    fprintf(ferr, "[%d]: Unexpected char '/'.\n", ++wrongnum);
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
                continue;
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
                fprintf(ferr, "[%d]: Wrong syntax.\n", ++wrongnum);
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
                if(InStrList(keyword, readbuf, keylistsize))
                    tokentype = Token::Keyword;
                else
                    tokentype = Token::Identifier;
                state = Scanner::Normal;
                retract();
            }
        }
        else if(state == Scanner::Annotation)
        {
            readbuf[strptr++] = curchar;
            if((formerchar == '/' && curchar != '\n') ||
                (formerchar == '*' && curchar != '*' && curchar != EOF)) {}
            else if(formerchar == '*' && curchar == '*' && TryNext('/'))
            {
                readbuf[strptr++] = '/';
                tokentype = Token::Annotation;
                state = Scanner::Normal;
            }
            else if(formerchar == '/' && (curchar == '\n' || TryNext(EOF)))
            {
                tokentype = Token::Annotation;
                state = Scanner::Normal;
            }
            else
                fprintf(ferr, "[%d]: Wrong annotation format.\n", ++wrongnum);
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
                bool match = false;
                int i, len = strptr;
                while(len)
                {
                    if(InStrList(signlist, readbuf, len, signlistsize))
                    {
                        match = true;
                        break;
                    }
                    len--;
                }
                if(match)
                {
                    tokentype = Token::Sign;
                    state = Scanner::Normal;
                    for(i = len; i < strptr; i++)
                    {
                        readbuf[i] = '\0';
                        retract();
                    }
                    retract();
                }
                else
                    fprintf(ferr, "[%d]: Unexpected sign \"%s\"!\n", ++wrongnum, readbuf);
            }
        }
        //here
        else if(state == Scanner::String)
        {
            if(curchar == EOF)
                fprintf(ferr, "[%d]: Wrong string format. Expecting a \' %c \'\n", ++wrongnum, formerchar);
            // printf("cur[1]: %c\n", curchar);
            readbuf[strptr++] = curchar;
            // printf("cur[1]: %c\n", curchar);
            if(formerchar == curchar)
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
            if(tokentype == Token::Annotation)
                fprintf(fdoc, readbuf);
            else
            {
                tokenlist->pushback(TokenInitVal(tokentype, readbuf));
                if(tokentype == Token::EndSymbol)
                    break;
            }
            memset(readbuf, 0, sizeof(readbuf));
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
    // while(!(EndOfFile && buffer[forward]))
    // while(forward != markend)
    // {
        // SkipSpace();
    read();
    // }
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
    curchar = buffer[forward - 1];
    retracted = true;
}

bool Scanner::TryNext(char ch)
{
    if(GetChar())
    {
        if(ch == 'd' && isdigit(curchar))
            return true;
        else if(curchar == ch)
            return true;
        retract();
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
    markend = -1;
    signlistsize = 0, keylistsize = 0, wrongnum = 0;
    while(signlist[signlistsize][0] != '\0')
        signlistsize++;
    while(keyword[keylistsize][0] != '\0')
        keylistsize++;
    InitSignCharset(signlist, signlistsize);
    tokenlist = TokenListInit();
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
    char *extension = GetExtension(file);
    char *errext = GetExtension(err);
    char *docext = GetExtension(doc);
    printf("fileext:%s, errext:%s\n", extension, errext);
    if(((extension[0] == 'c' && extension[1] == '\0') ||
        (extension[0] == 'c' && extension[1] == 'p' &&
        extension[2] == 'p' && extension[3] == '\0')) &&
        errext[0] == 't' && errext[1] == 'x' &&
        errext[2] == 't' && errext[3] == '\0' &&
        docext[0] == 't' && docext[1] == 'x' &&
        docext[2] == 't' && docext[3] == '\0')
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
    // std::list<const Token*>::iterator it;
    // for(it = tokenlist.begin(); it != tokenlist.end(); it++)
    // {
    //     const Token* cur = *it;
    //     printf("Type: %-18s Value: %s\n",
    //         tmp->type == Token::Macro ? "Macro" :
    //         tmp->type == Token::Keyword ? "Keyword" :
    //         tmp->type == Token::Int ? "Int" :
    //         tmp->type == Token::Float ? "Float" :
    //         tmp->type == Token::Identifier ? "Identifier" :
    //         tmp->type == Token::Annotation ? "Annotation" :
    //         tmp->type == Token::Sign ? "Sign" :
    //         tmp->type == Token::String ? "String" :
    //         tmp->type == Token::EndSymbol ? "EndSymbol" :
    //         "Wrong syntax!", tmp->value);
    // }
    Token* cur = tokenlist->head->next;
    int num = 0;
    while(cur != NULL)
    {
        printf("Type: %-18s Value: %s\n",
            cur->type == Token::Macro ? "Macro" :
            cur->type == Token::Keyword ? "Keyword" :
            cur->type == Token::Int ? "Int" :
            cur->type == Token::Float ? "Float" :
            cur->type == Token::Identifier ? "Identifier" :
            cur->type == Token::Annotation ? "Annotation" :
            cur->type == Token::Sign ? "Sign" :
            cur->type == Token::String ? "String" :
            cur->type == Token::EndSymbol ? "EndSymbol" :
            "Wrong syntax!", cur->value);
        cur = cur->next;
        num++;
    }
    printf("Amount of token: %d\n", num);
}
// void Destruct()
// {
//     KeyWord.clear();
//     KeyWord.shrink_to_fit();
// }
void PrintUsage()
{
    printf("Usage:\nsgcc file.cpp err.txt doc.txt\n");
    exit(-2);
}
int main(int argc, char* argv[])
{
    int i = 01;
    printf("i = %d\n", i);
    if(argc<4)
        PrintUsage();
    else
    {
        file = argv[1];
        err = argv[2];
        doc = argv[3];
    }
    if(!ProcessInput())
    {
        fprintf(stderr, "Wrong inputfile format.\n");
        return -1;
    }
    Scanner sc = Scanner(file, err, doc);
    sc.Tokenizer();
    fclose(sc.fin);
    fclose(sc.ferr);
    fclose(sc.fdoc);
}