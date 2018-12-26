#include "define.h"
#include "token.h"
#include "scanner.h"
#include "grammar.h"
#include "parser.h"
int stack[kStackSize];
int recordstack[kStackSize];
// int curstack[kProOriSize];
// int llptr, numptr, recptr, indexptr;
int* wordlist[kSetPerSize];
// int topptr, 
int wordgroupsum;
int lltable[kProOriSize][kSetPerSize][kProLen];
int first[kProOriSize][kSetPerSize], follow[kProOriSize][kSetPerSize];
int firstplus[kProSize][kSetPerSize];
// int empfirst[kProSize];
int merge = 0;
char identifiersym[] = "identifier";
char constantsym[] = "constant";
char stringsym[] = "string-literal";
ChosenPro *prohead, *curpro;
// char eof[] = "eof";
int main(int argc, char* argv[])
{
    // int sum = 0;
    // for(int i = 0; i < tablex; i++)
    // {
    //     int code = table[i][0] - extraend;
    //     if(follow[code][0] == -1)
    //         sum++;
    // }
    // printf("ntersize = %d , sum = %d\n", ntersize, sum);
    // ProcessFile(argc, argv);
    InitGrammar();
    BuildLLTable();
    PrintLLTable();
    Parser(argc, argv);
}

ChosenPro *CreateNewPro()
{
    ChosenPro *newpro = (ChosenPro *)malloc(sizeof(ChosenPro));
    memset(newpro->next, -1, sizeof(newpro->next));
    newpro->pa = NULL;
    return newpro;
}

ChosenPro* CreateNewPro(int line, ChosenPro *pa)
{
    ChosenPro *newpro = CreateNewPro();
    newpro->line = line;
    newpro->pa = pa;
    Push(pa->next, line);
    return newpro;
}

ChosenPro* FreePro(ChosenPro *target)
{
    ChosenPro *pa = target->pa;
    int i, tl = target->line;
    free(target);
    target = NULL;
    for(i = 0; pa->next[i] != tl && pa->next[i] != -1; i++) {}
    pa->next[i] = -1;
    while(pa->next[i+1] != -1)
        pa->next[i] = pa->next[i+1];
    pa->next[i] = pa->next[i+1];
    return pa;
}

int PopNext(ChosenPro *target)
{
    int i = 0, temp;
    while(target->next[i+1] != -1)
        i++;
    temp = target->next[i];
    target->next[i] = -1;
    return temp;
}

int NextPos(ChosenPro *target)
{
    int i = 0;
    while(target->next[i] != -1)
        i++;
    return i;
}

// void Push(int* sta, int* ptr, int value)
// {
//     sta[(*ptr)++] = value;
//     // *(ptr);
// }

void Push(int* sta, int value)
{
    int i = 0;
    printf("iii before : %d\n", sta[0]);
    while(sta[i] != -1)
    {
        printf("i before: %d\n", i);
        i++;
    }
    sta[i] = value;
    // *(ptr);
}

void PushLineToStack(int *sta, int x)
{
    int k = SizeOfArr(table[x]);
    int indexemp = FindTerIndex(empsym);
    // Push(frontinsta, &numptr, -2);
    printf("[4]Choosing table %d :\n", x);
    for(int i = 0; table[x][i] != -1; i++)
        printf("%s   ", factor[table[x][i]]);
    printf("\n");
    printf("================end4=============\n\n\n");
    while(--k)
    {
        int q;
        if((q = table[x][k]) != indexemp)
        {
            Push(sta, q);
            int hhh = 0;
        }
    }
    printf("**********************************\n");
    for(int i = 0; sta[i] != -1; i++)
        printf("%s   ", factor[sta[i]]);
    printf("\n");
    printf("***************end5***************\n\n\n");    
}

int Pop(int* sta)
{
    int i = 0;
    while(sta[i+1] != -1)
        i++;
    int temp = sta[i];
    sta[i] = -1;
    return temp;
}

int Front(int* sta)
{
    int i = 0;
    while(sta[i+1] != -1)
        i++;
    return sta[i];
}

int IsEmpty(int* arr)
{
    return arr[0] == -1;
}

int TransWord(Token* word)
{
    if(word == NULL)
        return FindTerIndex(eofsym);
    else if(word->type == Type::Keyword
        || word->type == Type::Operator)
        return FindTerIndex(word->value);
    else if(word->type == Type::Number
        || word->type == Type::Int
        || word->type == Type::Float)
        return FindTerIndex(constantsym);
    else if(word->type == Type::Identifier)
        return FindTerIndex(identifiersym);
    else if(word->type == Type::String)
        return FindTerIndex(stringsym);
    else
        return -1;
}

int NextToken(Scanner* tokensc, int index)
{
    if(index >= wordgroupsum * kMaxBuf)
    {
        int j = 0, k = 0;
        Token* token = NextWord(tokensc);
        // 暂时没做预处理
        while(token->type == Type::Macro)
            token = NextWord(tokensc);
        wordlist[wordgroupsum] = (int*)malloc(sizeof(int)*kMaxBuf);
        for(j = 0; j < kMaxBuf; j++)
            wordlist[wordgroupsum][j] = -1;
        while(k < kMaxBuf && token != NULL)
        {
            PrintToken(token);
            // int a = TransWord(token);
            wordlist[wordgroupsum][k++] = TransWord(token);
            // printf("wordlist[%d][%d] = %d\n", wordgroupsum, k - 1, wordlist[wordgroupsum][k-1]);
            token = NextWord(tokensc);
        }
        wordgroupsum++;
    }
    int x = index/kMaxBuf, y = index%kMaxBuf;
    if(wordlist[x][y] != -1)
        return wordlist[index/kMaxBuf][index%kMaxBuf];
    return FindTerIndex(eofsym);
}

int HasNext(int* numstack, int* recordstack, int* indexstack, int* index, int* token)
{
    int llnext = -1;
    while(llnext == -1 && (curpro != prohead || !SizeOfArr(curpro->next)))
    {
        curpro = FreePro(curpro);
        llnext = Pop(curpro->next);
    }
    if(llnext != -1)
    {
        curpro = CreateNewPro(llnext, curpro);
        printf("lltable next : %d\n", llnext);
        int i, num = Pop(numstack);
        while(!IsEmpty(stack))
            Pop(stack);
        for(i = 0; i < num; i++)
        {
            int rec = Pop(recordstack);
            Push(stack, rec);
        }
        *(token) = Pop(indexstack);
        *(index) = Pop(indexstack);
        int hhh = 0;
        printf("[1111111111]stack: ");
        while(stack[hhh] != -1)
            printf("%s  ", factor[stack[hhh++]]);
        printf("\n");
        PushLineToStack(stack, llnext);
        return 1;
    }
    return 0;
}

int ExpandFocus(Scanner* tokensc)
{
    int numstack[kProOriSize];
    int indexstack[kProOriSize];
    int indexeof = FindTerIndex(eofsym), indexsta = FindNonIndex(stasym);
    int indexemp = FindTerIndex(empsym);
    int index = 0, focus = Front(stack);
    int token = NextToken(tokensc, index);
    memset(numstack, -1, sizeof(numstack));
    memset(indexstack, -1, sizeof(indexstack));
    memset(recordstack, -1, sizeof(recordstack));
    while(focus != indexeof || token != indexeof)
    {
        if(!IsNonTer(focus) || focus == indexeof)
        {
            if(focus == token)
            {
                Pop(stack);
                token = NextToken(tokensc, ++index);
                printf("NEW TOKEN : %s\n", factor[token]);
            }
            else
            {
                if(!HasNext(numstack, recordstack, indexstack, &index, &token))
                    return 0;
            }
        }
        else
        {
            int llindex = 0;
            if(lltable[focus-extraend][token][llindex] != -1)
            {
                Pop(stack);
                int sizell = SizeOfArr(lltable[focus-extraend][token]);
                int k, p = lltable[focus-extraend][token][llindex];
                if(sizell != 1)
                {
                    llindex++;
                    for(; lltable[focus-extraend][token][llindex] != -1; llindex++)
                    {
                        int next = lltable[focus-extraend][token][llindex];
                        Push(curpro->next, next);
                        // 有n种可能就保存n-1次状态
                        int k = SizeOfArr(stack);
                        Push(numstack, k);
                        Push(indexstack, index);
                        Push(indexstack, token);
                        while(--k >= 0)
                        {
                            Push(recordstack, stack[k]);
                        }
                    }
                }
                curpro = CreateNewPro(p, curpro);
                PushLineToStack(stack, p);
            }
            else 
            {
                if(!HasNext(numstack, recordstack, indexstack, &index, &token))
                    return 0;
            }
        }
        focus = Front(stack);
    }
    return 1;
}

void PrintMatchLine()
{
    int i = 0, time = 0, lines[kProOriSize];
    memset(lines, -1, sizeof(lines));
    while(curpro != prohead)
    {
        lines[i++] = curpro->line;
        curpro = curpro->pa;
    }
    printf("Table:\n");
    while(--i >= 0)
    {
        printf("%d   ", lines[i]);
        if(time && time % 9 == 0)
            printf("\n");
        time++;
    }
}

int ExpandMatch(Scanner* tokensc, int* lines)
{
    int indexeof = FindTerIndex(eofsym);
    int indexsta = FindNonIndex(stasym);
    int indexemp = FindTerIndex(empsym);
    int index = 0, lptr = 0;
    int focus = Front(stack);
    int token = NextToken(tokensc, index);
    while(focus != indexeof || token != indexeof)
    {
        if(!IsNonTer(focus) || focus == indexeof)
        {
            if(focus == token)
            {
                Pop(stack);
                token = NextToken(tokensc, ++index);
                printf("NEW TOKEN : %s\n", factor[token]);
            }
            else
                fprintf(stderr, "ter error???\n");
        }
        else
        {
            int p = lines[lptr++];
            PushLineToStack(stack, p);
            else 
                fprintf(stderr, "NON-ter error???\n");
        }
        focus = Front(stack);
    }
    return 1;
}

void Parser(int argc, char* argv[])
{
    Scanner* tokensc = InitLexAnaly(argc, argv);
    if(tokensc)
    {
        memset(stack, -1, sizeof(stack));
        memset(wordlist, 0, sizeof(wordlist));
        // memset(curstack, -1, sizeof(curstack));
        wordgroupsum = 0;
        // topptr = 0;
        int indexeof = FindTerIndex(eofsym), indexsta = FindNonIndex(stasym);
        int indexemp = FindTerIndex(empsym);
        prohead = CreateNewPro();
        curpro = prohead;
        Push(stack, indexeof);
        Push(stack, indexsta);
        // int it = 0;
        ExpandFocus(tokensc);
        Push(stack, indexeof);
        Push(stack, indexsta);
        // PrintMatchLine();
    }
}


int InSet(int fac, int set[])
{
    int i;
    for(i = 0; set[i] != -1; i++)
        if(set[i] == fac)
            return 1;
    return 0;
}

void InitLLTable()
{
    int i, j;
    memset(lltable, -1, sizeof(lltable));
    for(i = 0; i< ntersize; i++)
    {
        //62
        for(j = ioft[i]; table[j][0] == i + extraend; j++)
        {
            int k, ter, indexeof;
            indexeof = FindTerIndex(eofsym);
            for(k = 0; (ter = firstplus[j][k]) != -1; k++)
            {
                int size = SizeOfArr(lltable[i][ter]);
                if(!InSet(j, lltable[i][ter]))
                    lltable[i][ter][size] = j;
                // lltable[i][ter] = j;
            }
            // 什么产生式的firstplus集合有eof？->start
            if(InSet(indexeof, firstplus[i]))
            {
                int size = SizeOfArr(lltable[i][ter]);
                lltable[i][indexeof][size] = j;
            }
        }
    }
}

void PrintFirstPlus()
{
    // FILE* llt = fopen("lltable.txt", "w");
    int i, j;
    for(i = 0; i< ntersize; i++)
    {
        for(j = ioft[i]; table[j][0] == i + extraend; j++)
        {
            int k, ter;
            for(k = 0; k < kSetPerSize; k++)
            {
                if((ter = firstplus[i][k]) != -1)
                    printf("(%s, %s) -> %d\n", factor[i+extraend], factor[ter], j);
            }
            // 什么产生式的firstplus集合有eof？
            // if(eof in firstplus[j])，没处理
        }
    }
}

void PrintLLTable()
{
    FILE* llt = fopen("lltable.txt", "w");
    int i, j, k;
    for(i = 0; i< ntersize; i++)
    {
        for(j = 0; j < extraend; j++)
        {
            int size = SizeOfArr(lltable[i][j]);
            for(k = 0; k < size; k++)
            {
                if(lltable[i][j][k] != -1)
                    fprintf(llt, " [%d] (%s, %s) -> %d\n",
                        k, factor[i+extraend], factor[j], lltable[i][j][k]);
            }
        }
    }
}

void BuildLLTable()
{
    BuildSets();
    InitLLTable();
}

void BuildSets()
{
    BuildFirst();
    BuildFollow();
    BuildFirstPlus();
}


// 将有序集合sup添加到有序集合ori中，且添加后的ori集合依然有序
int Merge(int ori[], int sup[])
{
    // 3342
    // printf("Merging[%d]\n", merge++);
    int ii = 0, iii = 0;
    int i, k = 0, start1 = 0, start2 = 0, end1, end2;
    int tempori[kProOriSize];
    end1 = SizeOfArr(ori);
    end2 = SizeOfArr(sup);
    memset(tempori, -1, sizeof(tempori));
    while(start1 < end1 && start2 < end2)
    {
        if(ori[start1] != sup[start2])
            tempori[k++] = ori[start1] > sup[start2] ? sup[start2++] : ori[start1++];
        else
        {
            tempori[k++] = ori[start1++];
            start2++;
        }
    }
    while(start1 < end1)
    {
        if(tempori[k-1] != ori[start1])
            tempori[k++] = ori[start1];
        start1++;
    }
    while(start2 < end2)
    {
        if(tempori[k-1] != sup[start2])
            tempori[k++] = sup[start2];
        start2++;
    }
    for(i = 0; i < k; i++)
        ori[i] = tempori[i];
    if(i > end1)
        return 1;
    return 0;
}

void Swap(int* a, int* b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

void QuickSort(int arr[], int start, int end)
{
    if(start >= end)
        return;
    int mid = arr[end], left = start, right = end - 1;
    while(left < right)
    {
        while(arr[left] < mid && left < right)
            left++;
        while(arr[right] >= mid && left < right)
            right--;
        Swap(&arr[left], &arr[right]);
    }
    if(arr[left] >= arr[end])
        Swap(&arr[left], &arr[end]);
    else
        left++;
    if(left)
        QuickSort(arr, start, left - 1);
    QuickSort(arr, left + 1, end);
}


// 减去集合中某元素，并重排集合
void SubEle(int* set, int element)
{
    int k = 0;
    while(set[k] != -1 && set[k] != element)
        k++;
    if(set[k] == element)
    {
        int end = k;
        set[k] = -1;
        while(set[end] != -1)
            end++;
        QuickSort(set, k, end - 1);
    }
}

// 如果用函数递归调用来实现会出现右递归（依赖）导致死循环的问题
void BuildFirst()
{
    memset(first, -1, sizeof(first));
    // memset(empfirst, 0, sizeof(empfirst));
    int changing = 1;
    while(changing)
    {
        changing = 0;
        int x, tempfirst[kProOriSize];
        for(x = 0; x < tablex; x++)
        {
            int i, j = 0, fac = table[x][1], indexemp;
            int target = table[x][0];
            memset(tempfirst, -1, sizeof(tempfirst));
            if(!IsNonTer(fac))
            {
                int terfirst[2] = {fac, -1};
                Merge(tempfirst, terfirst);
                Merge(first[target-extraend], tempfirst);
                continue;
            }
            else
                Merge(tempfirst, first[fac-extraend]);
            indexemp = FindTerIndex(empsym);
            // if(empfirst[fac-extraend])
            if(InSet(indexemp, first[fac-extraend]))
                SubEle(tempfirst, indexemp);
            i = 1;
            while(InSet(indexemp, first[fac-extraend]) && table[x][i+1] != -1)
            {
                fac = table[x][++i];
                if(!IsNonTer(fac))
                {
                    int terfirst[2] = {fac, -1};
                    Merge(tempfirst, terfirst);
                }
                else
                    Merge(tempfirst, first[fac-extraend]);
                SubEle(tempfirst, indexemp);
            }
            if(table[x][i+1] == -1 && InSet(indexemp, first[fac-extraend]))
            {
                int empsub[2] = {indexemp, -1};
                Merge(tempfirst, empsub);
                // empfirst[target-extraend] = 1;
            }
            if(!changing)
                changing = Merge(first[target-extraend], tempfirst);
            else
                Merge(first[target-extraend], tempfirst);
        }
    }
}

void Print(int set[][kSetPerSize])
{
    int i, j;
    for(i = 0; i < ntersize; i++)
    {
        printf("Set of %s:\n", factor[i+extraend]);
        for(j = 0; set[i][j] != -1; j++)
            printf("%s   ", factor[set[i][j]]);
        printf("\n\n\n\n\n");
    }
}

void DeleteEmp(int* set)
{
    int p = 0, end;
    int indexemp = FindTerIndex(empsym);
    while(set[p] != -1 && set[p] != indexemp)
        p++;
    end = p;
    while(set[end] != -1)
        end++;
    if(set[p] == indexemp)
    {
        set[p] = kProSize;
        QuickSort(set, p, end-1);
        set[end-1] = -1;
    }
}

void BuildFollow()
{
    int start, changing = 1, traptr = 0;
    int indexemp = FindTerIndex(empsym);
    memset(follow, -1, sizeof(follow));
    start = FindNonIndex(stasym);
    follow[start-extraend][0] = FindTerIndex(eofsym);
    while(changing)
    {
        changing = 0;
        int x;
        for(x = 0; x < tablex; x++)
        {
            int i, k, a = table[x][0], trailer[kProOriSize*kProLen];
            memset(trailer, -1, sizeof(trailer));
            Merge(trailer, follow[a-extraend]);
            k = SizeOfArr(table[x]) - 1;
            // while(table[x][k+1] != -1)
            //     k++;
            for(i = k; i > 0; i--)
            {
                int curcode = table[x][i];
                if(IsNonTer(curcode))
                {
                    if(!changing)
                    {
                        changing = Merge(follow[curcode-extraend], trailer);
                    }
                    else
                    {
                        Merge(follow[curcode-extraend], trailer);
                    }
                    // if(empfirst[curcode-extraend])
                    if(InSet(indexemp, first[curcode-extraend]))
                    {
                        Merge(trailer, first[curcode-extraend]);
                        DeleteEmp(trailer);
                    }
                    else
                    {
                        memset(trailer, -1, sizeof(trailer));
                        Merge(trailer, first[curcode-extraend]);
                    }
                }
                else
                {
                    memset(trailer, -1, sizeof(trailer));
                    int terfollow[2] = {curcode, -1};
                    Merge(trailer, terfollow);
                }
            }
        }
    }
}

void BuildFirstPlus()
{
    int i;
    memset(firstplus, -1, sizeof(firstplus));
    for(i = 0; i < tablex; i++)
    {
        int leftpro = table[i][0], firfac = table[i][1], indexemp;
        indexemp = FindTerIndex(empsym);
        if(!IsNonTer(firfac))
        {
            int tempfac[2] = {firfac, -1};
            Merge(firstplus[i], tempfac);
        }
        else
            Merge(firstplus[i], first[firfac-extraend]);
        // if(firfac == indexemp || (IsNonTer(firfac) && empfirst[firfac-extraend]))
        if(firfac == indexemp || (IsNonTer(firfac) && InSet(indexemp, first[firfac-extraend])))
        {
            // if(leftpro == 176)
            //     printf("duck\n");
            Merge(firstplus[i], follow[leftpro-extraend]);
        }
    }
}

void PrintPlus()
{
    int i, j;
    for(i = 0; i < tablex; i++)
    {
        int leftpro = table[i][0], firfac = table[i][1];
        printf("NO Condition\n");
        printf("%s -> %s:\n", factor[leftpro], factor[firfac]);
        for(j = 0; firstplus[i][j] != -1; j++)
            printf("%s   ", factor[firstplus[i][j]]);
        printf("\n\n");
    }
}
