#ifndef TREE_H
#define TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h> // va_list 

typedef struct Node{
    int terminal;//是否为终结符
    int lnumber;//行号
    char *lextype;//词法类型名称
    char *strcode;//存储词素（Lexeme）的字符串形式
    union {
        float fval;
        unsigned int uval;
    };//存储数值类型的常量值

    struct Node* bros;
    struct Node* kids;

} Node;

Node* CreateNode(int t, int l, char *lex, char *s);

void BuildTree(Node* r, int cnt, ...);

void PrintTree(Node* r, int prefix);

extern Node* root;
#endif
