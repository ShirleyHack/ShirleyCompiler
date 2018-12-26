 #ifndef _DEFINE_H_
#define _DEFINE_H_
#define kStrBuf 100
#define kFileName 105
#define kMaxBuf 100
#define kLEnd 40
#define kMiddle 41
#define kREnd 81
#define kStart 0
#define kTokenLen 30
#define kTerSize 105
#define kMaxKey 50
#define kProSize 2000
#define kProOriSize 300
#define kProLen 12
#define kProSer 28
#define kStackSize 5000
#define kSetPerSize 150

enum Type
{
    None = 0 , Keyword, Number, Identifier, Operator,
    Annotation, String, EndSymbol, Macro, Int, Float,
    HeadFile
    // RegEx, Space, NewLine
    // keyword, num->constant, op->ch, string->string-literal, ?End?, 
};
enum State
{
    Normal, Sign, Anno, Str, Id,
    Num, Define
    // RegEx, Space,
};
struct ChosenPro
{
    int line;
    int next[10];
    struct ChosenPro *pa;
};
typedef struct ChosenPro ChosenPro;
#endif