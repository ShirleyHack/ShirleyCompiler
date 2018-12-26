#include <cctype>
#include <unordered_set>
#include "sign.h"
#include "token.h"
#include "externs.h"
#include "scanner.h"

char *file, *err, *doc;
char readbuf[kMaxBuf];
int signlistsize, keylistsize;
// markend, wrongnum, forwardptr;curchar, 
// bool EndOfFile;
// bool lread, rread, retracted;
// TokenList* tokenlist;
unordered_set<char> signcharset;
const char* signlist[]=
{
    "[", "]", "(", ")", "{",
    "}", ".", "->", "++", "--",
    "&", "*", "+", "-", "~",
    "!", "/", "%", "<<", ">>",
    "<", ">", "<=", ">=", "==",
    "!=", "^", "|", "&&", "||",
    "?", ":", ";", "...", "=",
    "*=", "/=", "%=", "+=", "-=",
    "<<=", ">>=", "&=", "^=", "|=",
    ",", "#", "##", "<:", ":>",
    "<%", "%>", "%:", "%:%:",
    "\0"
};

const char* keyword[]=
{
	"_Alignas",
    "_Atomic",   "_Bool",     "_Complex",   "_Generic",
    "_Imaginary","_Noreturn", "_Static_assert", "_Thread_local",
    "alignof",   "auto",      "break",      "case",
    "char",      "const",     "continue",   "default",
    "do",        "double",    "else",       "enum",
    "extern",    "float",     "for",        "goto",
    "if",        "inline",    "int",        "long",
    "register",  "restrict",  "return",     "short",
    "signed",    "sizeof",    "static",     "struct",
    "switch",    "typedef",   "union",      "unsigned",
    "void",      "volatile",  "while",      "\0"
};

void FillBuffer(Scanner* sc)
{
    int i;
    if(!sc->lread)
    {
        sc->rread = 0;
        sc->lread = 1;
        for(i = 0; i < kLEnd; i++)
        {
            if((sc->buffer[i] = fgetc(sc->fin)) == EOF)
            {
                sc->EndOfFile = 1;
                sc->markend = i + 1;
                break;
            }            
        }
    }
    else if(sc->lread && !sc->rread)
    {
        sc->lread = 0;
        sc->rread = 1;
        for(i = kLEnd + 1; i < kLEnd + kLEnd + 1; i++)
        {
            if((sc->buffer[i] = fgetc(sc->fin)) == EOF)
            {
                sc->EndOfFile = 1;
                sc->markend = i + 1;
                break;
            }            
        }
    }
    else
        fprintf(sc->ferr, "[%d]: Failed to fill buffer", ++sc->wrongnum);
}

// 需要完善：return false能不能好看些
int GetChar(Scanner *sc)
{
    if(sc->forwardptr == sc->markend && sc->EndOfFile)
        return 0;
    sc->curchar = sc->buffer[sc->forwardptr];
    // printf("sc->forwardptr = %d\n", sc->forwardptr);
    if(sc->curchar == '\n')
        sc->linenum++;
    else if(isalnum(sc->curchar))
        sc->charnum++;
    if(sc->buffer[++sc->forwardptr] == EOF)
    {
        if(!sc->retracted && !sc->EndOfFile)
            FillBuffer(sc);
        if(sc->forwardptr == sc->markend && sc->EndOfFile) {}
        else if(sc->forwardptr == kLEnd)
            sc->forwardptr++;
        else if(sc->forwardptr == kREnd)
            sc->forwardptr = kStart;
    }
    if(sc->retracted)
        sc->retracted = 0;
    return 1;
}

//预处理头文件&宏定义
void PreProcess(char* sentence, int line)
{
    const char* includestr = "include";
    const char* definestr = "define";
    char* commandptr = NULL;
    int hascommand = 0;
    int cmdptr;
    commandptr = strstr(sentence, includestr);
    if(commandptr != NULL)
    {
        hascommand = 1;
        for(cmdptr = 7; *(commandptr + cmdptr) == ' ' ||
            *(commandptr + cmdptr) == '\t'; cmdptr++) {}
        //CreateNewToken(Type::Macro)
    }
}


Token* NextWord(Scanner* sc)
{
    int strptr = 0;
    int haspoint = 0;
    char formerchar = '\0';
    Type tokentype = Type::None;
    memset(readbuf, 0, sizeof(readbuf));
    while(GetChar(sc))
    {
        if(sc->state == State::Normal)
        {
            if(sc->curchar == '#')
            {
                readbuf[strptr++] = sc->curchar;
                sc->state = State::Define;
            }
            else if(IsIdentifierFirst(sc->curchar))
            {
                readbuf[strptr++] = sc->curchar;
                sc->state = State::Id;
            }
            else if(sc->curchar == '/')
            {
                readbuf[strptr++] = sc->curchar;
                if(TryNext(sc, '=') || TryNext(sc, 'd'))
                    sc->state = State::Sign;
                else if(TryNext(sc, '/'))
                {
                    formerchar = '/';
                    sc->state = State::Anno;
                }
                else if(TryNext(sc, '*'))
                {
                    sc->state = State::Anno;
                    formerchar = '*';
                }
                else
                    fprintf(sc->ferr, "[%d]: Unexpected char '/'.\n", ++sc->wrongnum);
            }
            else if(InCharSet(sc->curchar))
            {
                readbuf[strptr++] = sc->curchar;
                if((sc->curchar == '-' || sc->curchar == '+') && TryNext(sc, 'd'))
                    sc->state = State::Num;
                else
                    sc->state = State::Sign;
            }
            else if(isdigit(sc->curchar))
            {
                readbuf[strptr++] = sc->curchar;
                sc->state = State::Num;
            }
            else if(sc->curchar == '\"' || sc->curchar == '\'')
            {
                readbuf[strptr++] = sc->curchar;
                formerchar = sc->curchar;
                sc->state = State::Str;
            }
            else if(sc->curchar == ' ' || sc->curchar == '\t' ||
                sc->curchar == '\n')
            {
                continue;
            }
            // else if()
            // {
            //     tokentype = Type::NewLine;
            // }
            else if(sc->curchar == EOF)
            {
                readbuf[strptr++] = sc->curchar;
                tokentype = Type::EndSymbol;
            }
            else
                fprintf(sc->ferr, "[%d]: Wrong syntax.\n", ++sc->wrongnum);
        }
        else if(sc->state == State::Define)
        {
            readbuf[strptr++] = sc->curchar;
            if(sc->curchar == '\n')
            {
                // 相当于忽视换行符
                tokentype = Type::Macro;
                sc->state = State::Normal;
            }
        }
        else if(sc->state == State::Id)
        {
            if(InIdentifierNotSpecific(sc->curchar))
                readbuf[strptr++] = sc->curchar;
            else
            {
                if(InStrList(keyword, readbuf, keylistsize))
                    tokentype = Type::Keyword;
                else
                    tokentype = Type::Identifier;
                sc->state = State::Normal;
                retract(sc);
            }
        }
        else if(sc->state == State::Anno)
        {
            readbuf[strptr++] = sc->curchar;
            if((formerchar == '/' && sc->curchar != '\n') ||
                (formerchar == '*' && sc->curchar != '*' && sc->curchar != EOF)) {}
            else if(formerchar == '*' && sc->curchar == '*' && TryNext(sc, '/'))
            {
                GetChar(sc);
                readbuf[strptr++] = sc->curchar;
                tokentype = Type::Annotation;
                sc->state = State::Normal;
            }
            else if(formerchar == '/' && (sc->curchar == '\n' || TryNext(sc, EOF)))
            {
                // 相当于忽视换行符
                tokentype = Type::Annotation;
                sc->state = State::Normal;
            }
            else
                fprintf(sc->ferr, "[%d]: Wrong annotation format.\n", ++sc->wrongnum);
        }
        else if(sc->state == State::Num)
        {
            if(isdigit(sc->curchar))
                readbuf[strptr++] = sc->curchar;
            else if(sc->curchar == '.' && haspoint == 0)
            {
                readbuf[strptr++] = sc->curchar;
                haspoint = 1;
            }
            else
            {
                if(haspoint)
                    tokentype = Type::Float;
                else
                    tokentype = Type::Int;
                sc->state = State::Normal;
                retract(sc);
            }
        }
        else if(sc->state == State::Sign)
        {
            if(InCharSet(sc->curchar))
                readbuf[strptr++] = sc->curchar;
            else
            {
                int match = 0;
                int i, len = strptr;
                while(len)
                {
                    if(InStrList(signlist, readbuf, len, signlistsize))
                    {
                        match = 1;
                        break;
                    }
                    len--;
                }
                if(match)
                {
                    tokentype = Type::Operator;
                    sc->state = State::Normal;
                    for(i = len; i < strptr; i++)
                    {
                        readbuf[i] = '\0';
                        retract(sc);
                    }
                    retract(sc);
                }
                else
                    fprintf(sc->ferr, "[%d]: Unexpected sign \"%s\"!\n", ++sc->wrongnum, readbuf);
            }
        }
        //here
        else if(sc->state == State::Str)
        {
            if(sc->curchar == EOF)
                fprintf(sc->ferr, "[%d]: Wrong string format. Expecting a \' %c \'\n", ++sc->wrongnum, formerchar);
            // printf("cur[1]: %c\n", sc->curchar);
            readbuf[strptr++] = sc->curchar;
            // printf("cur[1]: %c\n", sc->curchar);
            if(formerchar == sc->curchar)
            {
                tokentype = Type::String;
                sc->state = State::Normal;
            }
            // GetChar(sc);
        }
        // else if(sc->state == State::RegEx)
        // {
        //     /*wait for supplement.*/
        // }
        // else if(sc->state == State::Space)
        // {

        // }
        if(tokentype != Type::None)
        {
            if(tokentype == Type::Annotation)
                fprintf(sc->fdoc, readbuf);
            else
            {
                // retract(sc);
                return CreateNewToken(tokentype, readbuf);
            }
            break;
        }
    }
    return NULL;
}

void retract(Scanner *sc)
{
    if(sc->forwardptr == 0)
        sc->forwardptr = kREnd - 1;
    else
    {
        sc->forwardptr--;
        if(sc->forwardptr == kLEnd)
            sc->forwardptr--;
    }
    sc->curchar = sc->buffer[sc->forwardptr - 1];
    sc->retracted = 1;
}

int TryNext(Scanner *sc, char ch)
{
    if(GetChar(sc))
    {
        if(ch == 'd' && isdigit(sc->curchar))
            return 1;
        else if(sc->curchar == ch)
            return 1;
        retract(sc);
    }
    return 0;
}
// void Scanner::RefreshBuffer(int strptr)
// {
//     readbuf[strptr++] = curchar;
// }
// void Initialize()
// {
//     EndOfFile = false;
//     markend = -1;
//     signlistsize = 0, keylistsize = 0, sc->wrongnum = 0;
//     while(signlist[signlistsize][0] != '\0')
//         signlistsize++;
//     while(keyword[keylistsize][0] != '\0')
//         keylistsize++;
//     InitSignCharset(signlist, signlistsize);
//     // tokenlist = TokenListInit();
// }

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

int ProcessInput()
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
        return 1;
    return 0;
}

int InIdentifierNotSpecific(char ch)
{
    return isalnum(ch) || ch == '_';
}
int IsIdentifierFirst(char ch)
{
    return isalpha(ch) || ch == '_';
}
// void PrintTokenList()
// {
//     // std::list<const Token*>::iterator it;
//     // for(it = tokenlist.begin(); it != tokenlist.end(); it++)
//     // {
//     //     const Token* cur = *it;
//     //     printf("Type: %-18s Value: %s\n",
//     //         tmp->type == Type::Macro ? "Macro" :
//     //         tmp->type == Type::Keyword ? "Keyword" :
//     //         tmp->type == Type::Int ? "Int" :
//     //         tmp->type == Type::Float ? "Float" :
//     //         tmp->type == Type::Identifier ? "Identifier" :
//     //         tmp->type == Type::Annotation ? "Annotation" :
//     //         tmp->type == Type::Operator ? "Sign" :
//     //         tmp->type == Type::String ? "String" :
//     //         tmp->type == Type::EndSymbol ? "EndSymbol" :
//     //         "Wrong syntax!", tmp->value);
//     // }
//     Token* cur = tokenlist->head->next;
//     int num = 0;
//     while(cur != NULL)
//     {
//         printf("Type: %-18s Value: %s\n",
//             cur->type == Type::Macro ? "Macro" :
//             cur->type == Type::Keyword ? "Keyword" :
//             cur->type == Type::Int ? "Int" :
//             cur->type == Type::Float ? "Float" :
//             cur->type == Type::Identifier ? "Identifier" :
//             cur->type == Type::Annotation ? "Annotation" :
//             cur->type == Type::Operator ? "Sign" :
//             cur->type == Type::String ? "String" :
//             cur->type == Type::EndSymbol ? "EndSymbol" :
//             "Wrong syntax!", cur->value);
//         cur = cur->next;
//         num++;
//     }
//     printf("Amount of token: %d\n", num);
// }
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

Scanner* InitLexAnaly(int argc, char* argv[])
{
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
        return NULL;
    }
    // EndOfFile = false;
    // markend = -1;, wrongnum = 0
    signlistsize = 0, keylistsize = 0;
    while(signlist[signlistsize][0] != '\0')
        signlistsize++;
    while(keyword[keylistsize][0] != '\0')
        keylistsize++;
    InitSignCharset(signlist, signlistsize);
    Scanner *sc = CreateNewScanner(file, err, doc);
    FillBuffer(sc);
    // return &sc;
    // fclose(sc.fin);
    // fclose(sc.ferr);
    // fclose(sc.fdoc);
    return sc;
}

Scanner* CreateNewScanner()
{
    // keylistsize = 0;
    // while(keyword[keylistsize][0] != '\0')
    //     keylistsize++;

    Scanner* sc = (Scanner*)malloc(sizeof(Scanner));
    memset(sc->buffer, 0, sizeof(sc->buffer));
    sc->linenum = 0;
    sc->wordnum = 0;
    sc->charnum = 0;
    sc->markend = -1;
    sc->wrongnum = 0;
    sc->forwardptr = 0;
    sc->EndOfFile = 0;
    sc->lread = 0;
    sc->rread = 0;
    sc->retracted = 0;
    sc->curchar = '\0';
    sc->state = State::Normal;
    sc->buffer[kLEnd] = EOF;
    sc->buffer[kREnd] = EOF;
    return sc;
}

Scanner* CreateNewScanner(const char* filename)
{
    Scanner *sc = CreateNewScanner();
    memset(sc->buffer, 0 ,sizeof(sc->buffer));
    sc->buffer[kLEnd] = EOF;
    sc->buffer[kREnd] = EOF;
    sc->fin = fopen(filename, "r");
}
Scanner* CreateNewScanner(const char* filein,
    const char* fileerr, const char* filedoc)
{
    Scanner *sc = CreateNewScanner();
    memset(sc->buffer, 0 ,sizeof(sc->buffer));
    sc->buffer[kLEnd] = EOF;
    sc->buffer[kREnd] = EOF;
    sc->fin = fopen(filein, "r");
    sc->ferr = fopen(fileerr, "w");
    sc->fdoc = fopen(filedoc, "w");
}

int SizeOfArr(int* arr)
{
    int k = 0;
    while(arr[k] != -1)
        k++;
    return k;
}