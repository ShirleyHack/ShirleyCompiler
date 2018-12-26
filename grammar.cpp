#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "externs.h"
#include "scanner.h"
#include "grammar.h"
int sigchend, douchend, trichend, quachend, keyend, extraend;
int ntersize, ntercount;
int tablex, tabley, tableorix, tableoriy;
int ioftori[kProOriSize], ioft[kProSize];
int zstart, addcode = 0;
char addchar[kProLen];
char empsym[] = "$";
char stasym[] = "translation-unit";
char eofsym[] = "eof";
char* factor[kTerSize + kProSize];
// char* nonter[kProSize];
char nterbuf[kMaxBuf], wordbuf[kMaxBuf];
int tableori[kProSize][kProLen];
int table[kProSize][kProLen];
int nterrank[kProSize];
int extratersize;
FILE* gramafter;
// FILE* fmid = fopen("fmid.txt", "w");
const char* singlechar[] =
{
    "[", "]", "(", ")", "{",
    "&", "*", "+", "-", "~",
    "/", "%", "<", ">", "^",
    "?", ":", ";", "}", ".",
    "=", ",", "#", "!", "|",
    "$", "\0"
};
const char* doublechar[] =
{
    "->", "++", "--", "<<", ">>",
    "<=", ">=", "==", "!=", "&&",
    "||", "*=", "/=", "%=", "+=",
    "-=", "&=", "^=", "|=", "##",
    "<:", ":>", "<%", "%>", "%:",
    "\0"
};
const char* triplechar[] =
{
    "...", "<<=", ">>=", "\0"
};
const char* quadruplechar = "%:%:";
const char* extrater[] =
{
    "constant", "enumeration-constant", "identifier",
    "string-literal", "eof", "\0"
};

void InitOpTer()
{
    int i = 0, j = 0;
    char* newop;
    while(singlechar[j][0] != '\0')
    {
        newop = (char*)malloc(sizeof(2));
        strcpy(newop, singlechar[j]);
        factor[i++] = newop;
        j++;
    }
    sigchend = i;
    j = 0;
    while(doublechar[j][0] != '\0')
    {
        newop = (char*)malloc(sizeof(3));
        strcpy(newop, doublechar[j]);
        factor[i++] = newop;
        j++;
    }
    douchend = i;
    j = 0;
    while(triplechar[j][0] != '\0')
    {
        newop = (char*)malloc(sizeof(4));
        strcpy(newop, triplechar[j]);
        factor[i++] = newop;
        j++;
    }
    trichend = i;
    newop = (char*)malloc(sizeof(5));
    strcpy(newop, quadruplechar);
    factor[i++] = newop;
    quachend = i;
}

void MergeSortRec(char* origin[], char after[][kMaxKey], int start, int end)
{
    if(start >= end)
        return;
    int len = end - start, mid = (len >> 1) + start;
    int start1 = start, end1 = mid;
    int start2 = mid + 1, end2 = end;
    MergeSortRec(origin, after, start1, end1);
    MergeSortRec(origin, after, start2, end2);
    int i, k = start;
    // for(i = start; i < end; i++)
    //     memset(after[i], 0, sizeof(after[i]));
    while(start1 <= end1 && start2 <= end2)
    {
        if(strcmp(origin[start1], origin[start2]) < 0)
            strcpy(after[k++], origin[start1++]);
        else
            strcpy(after[k++], origin[start2++]);
    }
    while(start1 <= end1)
    {
        strcpy(after[k], origin[start1++]);
        k++;
    }
    while(start2 <= end2)
    {
        strcpy(after[k], origin[start2++]);
        k++;
    }
    for(k = start; k <= end; k++)
        strcpy(origin[k], after[k]);
}

void InsertWord(const char* origin[], char* after[], int len)
{
    int i;
    for(i = 0; i < len; i++)
    {
        after[i] = (char*)malloc(sizeof(kMaxKey));
        strcpy(after[i], origin[i]);
    }
}

void InitKeyTer()
{
    InsertWord(keyword, factor + quachend, keylistsize);
    keyend = quachend + keylistsize;
}

void InitExtraTer()
{
    InsertWord(extrater, factor + keyend, extratersize);
    extraend = keyend + extratersize;
}

void InitTer()
{
    InitOpTer();
    InitKeyTer();
    InitExtraTer();
    char tempter[kTerSize][kMaxKey];
    MergeSortRec(factor + quachend, tempter, 0, extraend - quachend - 1);
}

int BinarySearch(char* list[], int start, int end, char* cur)
{
    if(start > end)
        return -1;
    // char why[] = "type-qualifier-list";
    // if(strcmp(cur, why) == 0)
    // {
    //     printf("Here to start\n");
    // }
    int mid = start + (end - start) / 2;
    if(strcmp(list[mid], cur) > 0)
        return BinarySearch(list, start, mid - 1, cur);
    else if(strcmp(list[mid], cur) < 0)
        return BinarySearch(list, mid + 1, end, cur);
    else return mid;
}

int FindTerIndex(char* curter)
{
    int i = -1;
    int len = strlen(curter);
    if(len)
    {
        if(isalpha(curter[0]) || curter[0] == '_')
        {
            i = BinarySearch(factor + quachend, 0, extraend - quachend -1, curter);
            return i != -1 ? quachend + i : i; 
        }
        else
        {
            if(len == 1)
            {
                for(i = 0; i < sigchend; i++)
                {
                    if(factor[i][0] == curter[0])
                        return i;
                }
            }
            else if(len == 2)
            {
                for(i = sigchend; i < douchend; i++)
                {
                    if(strcmp(factor[i], curter) == 0)
                        return i;
                }
            }
            else if(len == 3)
            {
                for(i = douchend; i < trichend; i++)
                {
                    if(strcmp(factor[i], curter) == 0)
                        return i;
                }
            }
            else if(len == 4 && strcmp(factor[i], curter) == 0)
                return i;
        }
    }
    return -1;
}

int FindNonIndex(char* curnon)
{
    int result = BinarySearch(factor + extraend, 0, ntersize - 1, curnon);
    if(result == -1)
        return -1;
    return extraend + result;
}

int FindIndex(char* cur)
{
    int index = FindTerIndex(cur);
    // char emp[] = "$";
    // if(strcmp(cur, emp) == 0)
    //     printf("Here is the index of %s: %d\n", emp, index);
    if(index != -1 && index < extraend)
        return index;
    index = FindNonIndex(cur);
    if(index != -1 && index >= extraend)
        return index;
    return -1;
}

void GetNonTer(char* str)
{
    int i = 0;
    memset(nterbuf, 0, sizeof(nterbuf));
    while(isalpha(str[i]) || str[i] == '-' || str[i] == '_')
    {
        nterbuf[i] = str[i];
        i++;
    }
    if(str[i] == ':')
        i++;
}

void GetNonTer(Scanner *gr)
{
    int i = 0;
    memset(nterbuf, 0, sizeof(nterbuf));
    while(isalpha(gr->curchar) || gr->curchar == '-' || gr->curchar == '_')
    {
        nterbuf[i++] = gr->curchar;
        GetChar(gr);
    }
    if(gr->curchar == ':')
        GetChar(gr);
}

void SkipSpace(Scanner *gr)
{
    while(gr->curchar == '\n' || gr->curchar == ' ' || gr->curchar == '\t')
        GetChar(gr);
}

void SkipSpaceInLine(Scanner *gr)
{
    while(gr->curchar == ' ' || gr->curchar == '\t')
        GetChar(gr);
}

void StoreFactor(Scanner *gr)
{
    int i = 0;
    memset(wordbuf, 0, sizeof(wordbuf));
    SkipSpaceInLine(gr);
    if(gr->curchar == EOF)
        return;
    while(gr->curchar != ' ' && gr->curchar != '\n')
    {
        wordbuf[i++] = gr->curchar;
        if(gr->curchar == EOF || i>95)
        {
            printf("[%d]: %s\n", i, wordbuf);
        }
        GetChar(gr);
    }
    wordbuf[i] = '\0';

    // 判断是ter还是non
    int index = FindIndex(wordbuf);
    if(index != -1)
        tableori[tableorix][tableoriy++] = index;
    else
    {
        fprintf(stderr, "%s in (%d, %d)\n", wordbuf, tableorix, tableoriy);
    }
}

void ScanPro(Scanner *gr)
{
    FillBuffer(gr);
    while(GetChar(gr))
    {
        // printf("tableorix before: %d\n", tableorix);
        ScanANonTer(gr);
        // printf("tableorix after: %d\n", tableorix);
    } 
}

int MakeAddition()
{
    int len = strlen("z-addition") + 8;
    factor[extraend + ntersize] = (char*)malloc(sizeof(char)*len);
    strcpy(factor[extraend + ntersize], "z-addition");
    int i, num = addcode / 26;
    if(addchar[num] == '\0')
        addchar[num] = 'a';
    else if('a' <= addchar[num] && addchar[num] <= 'y')
        addchar[num] += 1;
    strcat(factor[extraend + ntersize], addchar); 
    addcode++;
    int temp = extraend + ntersize;
    ntersize++;
    return temp;
}

int EliminateDireLRecur(int start, int* endx)
{
    int i, newcode, recsum = 0, end = *(endx);
    int indexemp = FindTerIndex(empsym);
    // memset(rec, -1, sizeof(rec));
    for(i = start; i < end; i++)
    {
        if(tableori[i][0] == tableori[i][1])
        {
            recsum = 1;
            break;
        } 
    }
    if(recsum)
    {
        int q, first = -1;
        newcode = MakeAddition();
        for(i = start; i < end; i++)
        {
            if(tableori[i][0] == tableori[i][1])
            {
                if(first == -1)
                {
                    ioftori[newcode-extraend] = i;
                    first = 1;
                }
                tableori[i][0] = newcode;
                for(q = 1; tableori[i][q] != -1; q++)
                    tableori[i][q] = tableori[i][q+1];
                q--;
                tableori[i][q] = newcode;
            }
            else
            {
                q = SizeOfArr(tableori[i]);
                tableori[i][q] = newcode;
            }
        }
        tableori[end][0] = newcode;
        tableori[end][1] = indexemp;
        // printf("endx before:%d\n", *endx);
        (*endx)++;
        // printf("endx after:%d\n", *endx);
    }
    // return 0;
}

void ScanANonTer(Scanner *gr)
{
    GetNonTer(gr);
    int nter = FindNonIndex(nterbuf), startx = tableorix;
    ioftori[nter - extraend] = tableorix;
    while(1)
    {
        if(tableori[tableorix][0] != -1)
            tableorix++;
        tableoriy = 0;
        tableori[tableorix][tableoriy++] = nter;
        // 除掉行开头'|'
        if(gr->curchar == '|')
            GetChar(gr);
        while(gr->curchar != '\n' && gr->curchar != EOF)
        {
            StoreFactor(gr);
            SkipSpaceInLine(gr);
        }
        SkipSpace(gr);
        tableorix++;
        if(gr->curchar != '|')
        {
            if(gr->curchar != EOF)
                retract(gr);
            if(tableorix - startx > 1)
            {
                // printf("nonter : %s, start = %d, end = %d\n", factor[nter], startx, tableorix);
                EliminateDireLRecur(startx, &tableorix);
            }
            // for(int temp = startx; temp < tableorix; temp++)
            // {
            //     for(int jj = 0; tableori[temp][jj] != -1; jj++)
            //         fprintf(fmid, "%s  ", factor[tableori[temp][jj]]);
            //     fprintf(fmid, "\n");
            // }            
            // {
            //     endx = tableorix;
            //     ExtractLFactor(gr, startx, endx);
            // }
            return;
        }
    }
}

int IsNonTer(int index)
{
    if(index < extraend)
        return 0;
    return 1;
}

void InitGrammar()
{
    keylistsize = 0;
    extratersize = 0;
    memset(addchar, 0, sizeof(addchar));
    while(keyword[keylistsize][0] != '\0')
        keylistsize++;
    while(extrater[extratersize][0] != '\0')
        extratersize++;
    ntersize = 0, ntercount = 0;
    tableorix = 0, tableoriy = 0;
    tablex = 0, tabley = 0;
    // EndOfFile = false;
    // markend = -1, forwardptr = 0;
    memset(tableori, -1, sizeof(tableori));
    memset(table, -1, sizeof(table));
    memset(factor, 0, sizeof(factor));
    memset(ioftori, -1, sizeof(ioftori));
    memset(ioft, -1, sizeof(ioft));
    gramafter = fopen("after.txt", "w");
    // FILE* gramfile;
    // gramfile = fopen("grammar.txt", "r");
    InitTer();
    PreProcessNon();
    Scanner* gr = CreateNewScanner("grammar.txt");
    gr->ferr = fopen("err.txt", "w");
    // gr.FillBuffer();
    ScanPro(gr);
    ProcessPro();
    PrintPro();
    // FreeMemory();
}

// int main()
// {
//     InitGrammar();
//     // BuildTable();
// }

void ProcessPro()
{
    EliminateLRecur();
}

void PreProcessNon()
{
    int i, linenum = 0;
    char str[kMaxBuf];
    char tempfac[kProSize][kMaxKey];
    FILE* fin = fopen("grammar.txt", "r");
    memset(str, 0, sizeof(str));
    memset(nterrank, -1, sizeof(nterrank));
    while(fgets(str, kMaxBuf, fin) != NULL)
    {
        if(str[0] != '|')
        {
            GetNonTer(str);
            factor[extraend + ntersize] = (char*)malloc(sizeof(char)*strlen(str));
            strcpy(factor[extraend+ntersize], nterbuf);
            ntersize++;
        }
    }
    fclose(fin);
    MergeSortRec(factor + extraend, tempfac, 0, ntersize - 1);
    zstart = ntersize;
}

int InPopList(int popout[], int k)
{
    int i = 0;
    while(popout[i] != -1)
    {
        if(k == popout[i++])
            return 1;
    }
    return 0;
}

void Swap(proorder* a, proorder* b)
{
    proorder t;
    t.oriindex = a->oriindex;
    t.value = a->value;
    a->oriindex = b->oriindex;
    a->value = b->value;
    b->oriindex = t.oriindex;
    b->value = t.value;
}

void QuickSort(proorder* arr, int start, int end)
{
    // printf("In Quick\n");
    if(start >= end)
        return;
    int mid = arr[end].value;
    int left = start, right = end-1;
    while(left < right)
    {
        // printf("In Quick Loop 1:\nleft.val = %d, right.val = %d\n", arr[left].value, arr[right].value);
        while(arr[left].value < mid && left < right)
        {
            left++;
            // printf("In Quick Loop 2\n");
        }
        while(arr[right].value >= mid && right > left)
        {
            right--;
            // printf("In Quick Loop 3\n");
        }
        Swap(&arr[left], &arr[right]);
    }
    if(arr[left].value >= arr[end].value)
        Swap(&arr[left], &arr[end]);
    else
        left++;
    if(left)
        QuickSort(arr, start, left - 1);
    QuickSort(arr, left + 1, end);
}

void ExtractLFactor(int startx, int endx)
{
    // printf("FIRST:%s\n", factor[table[startx][0]]);
    if(endx - startx <= 1)
    {
        int code = table[startx][0];
        if(ioft[code-extraend] == -1)
            ioft[code-extraend] = startx;
        return;
    }
    int i, j, k, front = 0, end = 0, popnum, len, acc;
    int quene[kProSer], popout[kProSer], temp[kProSer*kProLen][kProLen];
    for(i = startx; i < endx; i++)
    {
        int size = 0;
        while(table[i][size] != -1)
            size++;
        memset(quene, -1, sizeof(quene));
        // k = endx - i;
        front = 0, end = 0;
        k = 0;
        for(j = i; j < endx; j++)
            quene[k++] = j;
        j = 1;

        while(k > 1 && j <= size)
        {
            // printf("In loop 2\n");
            popnum = 0;
            len = k;
            memset(popout, -1, sizeof(popout));
            for(acc = 0; acc < len; acc++)
            {
                // printf("In loop 3\n");
                if(table[quene[front]][0] == table[i][0]
                    && table[quene[front]][1] != table[i][0]
                    && table[quene[front]][j] == table[i][j])
                {
                    int temp = quene[front];
                    quene[front] = -1;
                    front = (front + 1) % kProSer;
                    end = (front + k - 1) % kProSer;
                    quene[end] = temp;
                }
                else
                {
                    if(table[quene[front]][j] == -1)
                        table[quene[front]][j] = FindTerIndex(empsym);
                    if(table[i][j] == -1)
                        table[i][j] = FindTerIndex(empsym);
                    int temp = quene[front];
                    quene[front] = -1;
                    front = (front + 1) % kProSer;
                    k--;
                    popout[popnum++] = temp;
                }
            }
            if(popnum && j > 1)
            {
                int newcode = MakeAddition();
                // char warning[] = "z-addition59";
                // if(strcmp(factor[newcode], warning) == 0)
                //     printf("WARNING!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                // ntersize++;
                // int indexemp = FindTerIndex(empsym);
                int p;

                for(p = 0; table[i][p] != -1; p++)
                {
                    table[endx][p] = table[i][p];
                }
                while(table[endx][p] != -1)
                    table[endx][p++] = -1;
                p = j;
                table[i][p++] = newcode;

                while(table[i][p] != -1)
                    table[i][p++] = -1;
                popout[popnum++] = endx;
                for(p = 0; p < kProSer; p++)
                {
                    if(quene[p] != -1 && quene[p] != i)
                        popout[popnum++] = quene[p];
                }

                for(p = 0; p < popnum; p++)
                {
                    // printf("In loop 4\n");
                    int q, x = popout[p];
                    table[x][0] = newcode;
                    for(q = 1; table[x][q+j-1] != -1; q++)
                    {
                        table[x][q] = table[x][q+j-1];
                    }
                    while(table[x][q] != -1)
                        table[x][q++] = -1;
                }
                endx++;
                tablex++;
                break;
            }
            j++;
            len -= popnum;
        }
    }
    proorder ord[kProSer*kProLen];
    int p;
    for(p = 0; p < kProSer*kProLen; p++)
    {
        // printf("In loop 5\n");
        ord[p].value = -1;
        ord[p].oriindex = -1;
    }
    for(p = startx; p < endx; p++)
    {
        // printf("In loop 6\n");
        ord[p-startx].oriindex = p;
        ord[p-startx].value = table[p][0];
    }
    QuickSort(ord, 0, endx - startx - 1);
    // printf("AfterQuick\n");
    memset(temp, -1, sizeof(temp));
    int tempx = 0;
    for(p = startx; p < endx; p++)
    {
        // printf("In loop 7\n");
        int tx = ord[p-startx].oriindex;
        for(j = 0 ; table[tx][j] != -1; j++)
            temp[tempx][j] = table[tx][j];
        tempx++;
    }
    int newcode, formercode = temp[0][0];
    if(ioft[formercode-extraend] == -1)
        ioft[formercode-extraend] = startx;
    for(p = 0; p < tempx; p++)
    {
        // if(startx+p == 217 || startx+p == 218)
        //     printf("In extra before[%d]: table[%d][%d] = %d\n", startx+p, p, j, table[p][j]);
        for(j = 0 ; temp[p][j] != -1; j++)
        {
            table[startx+p][j] = temp[p][j];
        }
        while(table[startx+p][j] != -1)
            table[startx+p][j++] = -1;
        if((newcode = table[startx+p][0]) != formercode)
        {
            if(ioft[newcode-extraend] == -1)
                ioft[newcode-extraend] = startx + p;
            formercode = newcode;
        }
        // if(startx+p == 217 || startx+p == 218)
        //     printf("In extra after[%d]: table[%d][%d] = %d\n", startx+p, p, j, table[p][j]);
        // printf("In loop 8\n");
    }
}

// void RerangeTableOri()
// {
//     int i, j, k, q;
//     int x = 0;
//     int size = ntersize;
//     int temptab[kProSize][kProLen];
//     int tempioft[kProOriSize];
//     memset(temptab, -1, sizeof(temptab));
//     memset(tempioft, -1, sizeof(tempioft));
//     for(i = 0; i < size; i++)
//     {
//         tempioft[i] = x;
//         for(j = ioftori[i]; tableori[j][0] == i + extraend; j++)
//         {
//             printf("j = %d\n", j);
//             for(k = ioftori[tableori[j][1]-extraend]; tableori[k][0] == tableori[j][1]; k++)
//             {
//                 printf("k = %d\n", k);
//                 int y = 0;
//                 for(q = 0; tableori[k][q] != -1; q++)
//                     temptab[x][y++] = tableori[k][q];
//                 x++;
//             }
//         }
//     }
//     for(i = 0; i < x; i++)
//     {
//         for(j = 0; temptab[i][j] != -1; j++)
//             tableori[i][j] = temptab[i][j];
//     }
//     for(i = 0; i < size; i++)
//         ioftori[i] = tempioft[i];
// }


void EliminateLRecur()
{
    int i, j;
    int size = ntersize;
    // RerangeTableOri();
    for(i = 0; i < size; i++)
    {
        // printf("%d:Eliminate\n", i);
        int startx = tablex, listrec[kProOriSize][kProLen], listnotrec[kProOriSize][kProLen];
        for(j = ioftori[i]; tableori[j][0] == i + extraend; j++)
        {
            if(tableori[j][1] < i + extraend && IsNonTer(tableori[j][1]))
            {
                int k, count = 0;
                int p, q, size, temp[kProLen];

                for(k = ioftori[tableori[j][1]-extraend]; tableori[k][0] == tableori[j][1]; k++)
                {
                    tabley = 0;
                    table[tablex][tabley++] = tableori[j][0];
                    for(q = 1; tableori[k][q] != -1; q++)
                        table[tablex][tabley++] = tableori[k][q];
                    for(q = 2; tableori[j][q] != -1; q++)
                        table[tablex][tabley++] = tableori[j][q];
                    tablex++;
                }
            }
            else
            {
                int k;
                for(k = 0; tableori[j][k] != -1; k++)
                    table[tablex][k] = tableori[j][k];
                tablex++;
                continue;
            }
        }
        int p = 0, q = 0, indexemp;
        // int temp[kProSer*6][kProLen];
        indexemp = FindTerIndex(empsym);
        // memset(temp, -1, sizeof(temp));
        int newcode = -1;
        memset(listrec, -1, sizeof(listrec));
        memset(listnotrec, -1, sizeof(listnotrec));
        for(j = startx; table[j][0] == i + extraend; j++)
        {
            // 如果是左递归rec
            if(table[j][0] == table[j][1])
            {
                int k;
                if(newcode == -1)
                    newcode = MakeAddition();
                // ntersize++;
                for(k = 1; table[j][k] != -1; k++)
                    listrec[p][k] = table[j][k+1];
                k--;
                table[j][k] = newcode;
                listrec[p][k] = newcode;
                listrec[p++][0] = newcode;
                // temp[++p][0] = newcode;
                // temp[p++][1] = indexemp;
            }
            else
            {
                int k;
                for(k = 1; table[j][k] != -1; k++)
                    listnotrec[q][k] = table[j][k];
                listnotrec[q++][0] = table[j][0];
            }
        }
        // if(newcode > -1)
        // {
        //     int k;
        //     for(k = 0; k < marknum; k++)
        //     {
        //         int q = 0, x = listnotrec[k];
        //         while(table[x][q] != -1)
        //             q++;
        //         table[x][q] = newcode;
        //     }
        // }
        if(p)
        {
            listrec[p][0] = newcode;
            listrec[p++][1] = indexemp;            
        }
        j = startx;
        for(p = 0; listnotrec[p][0] != -1; p++)
        {
            for(q = 0; listnotrec[p][q] != -1; q++)
            {
                table[j][q] = listnotrec[p][q];
            }
                
            table[j][q++] = newcode;
            while(table[j][q] != -1)
                table[j][q++] = -1;
            j++;
        }
        tablex = j;
        ExtractLFactor(startx, tablex);
        startx = tablex;
        for(p = 0; listrec[p][0] != -1; p++)
        {
            // if(tablex == 217 || tablex == 218)
            //     printf("In elimi before[%d]: table[%d][%d] = %d\n", tablex, p, j, table[p][j]);
            for(q = 0; listrec[p][q] != -1; q++)
            {
                table[tablex][q] = listrec[p][q];
            }
                
            while(table[tablex][q] != -1)
                table[tablex][q++] = -1;
            // if(tablex == 217 || tablex == 218)
            //     printf("In elimi after[%d]: table[%d][%d] = %d\n", tablex, p, j, table[p][j]);
            tablex++;
        }
        // tablex = j;
        ExtractLFactor(startx, tablex);
    }
}

// void EliminateLRecur()
// {
//     int i, j;
//     int size = ntersize;
//     // RerangeTableOri();

//     for(i = 0; i < size; i++)
//     {
//         // printf("%d:Eliminate\n", i);
//         int startx = tablex, listrec[kProOriSize][kProLen], listnotrec[kProOriSize][kProLen];
//         for(j = ioftori[i]; tableori[j][0] == i + extraend; j++)
//         {
//             if(tableori[j][1] < i + extraend && IsNonTer(tableori[j][1]))
//             {
//                 int k, count = 0;
//                 int p, q, size, temp[kProLen];

//                 for(k = ioftori[tableori[j][1]-extraend]; tableori[k][0] == tableori[j][1]; k++)
//                 {
//                     tabley = 0;
//                     table[tablex][tabley++] = tableori[j][0];
//                     for(q = 1; tableori[k][q] != -1; q++)
//                         table[tablex][tabley++] = tableori[k][q];
//                     for(q = 2; tableori[j][q] != -1; q++)
//                         table[tablex][tabley++] = tableori[j][q];
//                     tablex++;
//                 }
//             }
//             else
//             {
//                 int k;
//                 for(k = 0; tableori[j][k] != -1; k++)
//                     table[tablex][k] = tableori[j][k];
//                 tablex++;
//                 continue;
//             }
//         }
//         int p = 0, q = 0, indexemp;
//         // int temp[kProSer*6][kProLen];
//         indexemp = FindTerIndex(empsym);
//         // memset(temp, -1, sizeof(temp));
//         int newcode = -1;
//         memset(listrec, -1, sizeof(listrec));
//         memset(listnotrec, -1, sizeof(listnotrec));
//         for(j = startx; table[j][0] == i + extraend; j++)
//         {
//             // 如果是左递归rec
//             if(table[j][0] == table[j][1])
//             {
//                 int k;
//                 if(newcode == -1)
//                     newcode = MakeAddition();
//                 // ntersize++;
//                 for(k = 1; table[j][k] != -1; k++)
//                     listrec[p][k] = table[j][k+1];
//                 k--;
//                 table[j][k] = newcode;
//                 listrec[p][k] = newcode;
//                 listrec[p++][0] = newcode;
//                 // temp[++p][0] = newcode;
//                 // temp[p++][1] = indexemp;
//             }
//             else
//             {
//                 int k;
//                 for(k = 1; table[j][k] != -1; k++)
//                     listnotrec[q][k] = table[j][k];
//                 listnotrec[q++][0] = table[j][0];
//             }
//         }
//         // if(newcode > -1)
//         // {
//         //     int k;
//         //     for(k = 0; k < marknum; k++)
//         //     {
//         //         int q = 0, x = listnotrec[k];
//         //         while(table[x][q] != -1)
//         //             q++;
//         //         table[x][q] = newcode;
//         //     }
//         // }
//         if(p)
//         {
//             listrec[p][0] = newcode;
//             listrec[p++][1] = indexemp;            
//         }
//         j = startx;
//         for(p = 0; listnotrec[p][0] != -1; p++)
//         {
//             for(q = 0; listnotrec[p][q] != -1; q++)
//             {
//                 table[j][q] = listnotrec[p][q];
//             }
                
//             table[j][q++] = newcode;
//             while(table[j][q] != -1)
//                 table[j][q++] = -1;
//             j++;
//         }
//         tablex = j;
//         ExtractLFactor(startx, tablex);
//         startx = tablex;
//         for(p = 0; listrec[p][0] != -1; p++)
//         {
//             // if(tablex == 217 || tablex == 218)
//             //     printf("In elimi before[%d]: table[%d][%d] = %d\n", tablex, p, j, table[p][j]);
//             for(q = 0; listrec[p][q] != -1; q++)
//             {
//                 table[tablex][q] = listrec[p][q];
//             }
                
//             while(table[tablex][q] != -1)
//                 table[tablex][q++] = -1;
//             // if(tablex == 217 || tablex == 218)
//             //     printf("In elimi after[%d]: table[%d][%d] = %d\n", tablex, p, j, table[p][j]);
//             tablex++;
//         }
//         // tablex = j;
//         ExtractLFactor(startx, tablex);
//     }
// }


void PrintPro()
{
    int i, j;
    // printf("==================origin==================\n");
    // for(i = 0; i < tableorix; i++)
    // {
    //     printf("%s: \n|", factor[tableori[i][0]]);
    //     // printf("%s\n", factor[tableori[i][1]]);
    //     for(j = 1; tableori[i][j] != -1; j++)
    //     {
    //         printf("  %s", factor[tableori[i][j]]);
    //     }
    //     printf("\n");
    // }
    // printf("==========================================\n\n\n");
    // fprintf(gramafter, "==================after==================\n");
    // printf("==================after==================\n");
    for(i = 0; i < tablex; i++)
    {
        fprintf(gramafter, "%s: ", factor[table[i][0]]);
        // printf("%s: ", factor[table[i][0]]);
        // printf("%s\n", factor[table[i][1]]);
        for(j = 1; table[i][j] != -1; j++)
        {
            fprintf(gramafter, "  %s", factor[table[i][j]]);
            // printf("  %s", factor[table[i][j]]);
        }
        fprintf(gramafter, "\n");
        // printf("\n");
    }
    // fprintf(gramafter, "===================end===================\n");
    // printf("===================end===================\n");
    fclose(gramafter);
    printf("orisize=%d\nsize=%d\n", tableorix, tablex);
}

void FreeMemory()
{
    for(int i = 0; factor[i] != NULL; i++)
        free(factor[i]);
}