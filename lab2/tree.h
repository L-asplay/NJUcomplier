#ifndef TREE_H
#define TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h> // va_list 

typedef struct Node{
    int terminal;
    int lnumber;
    char *lextype;
    char *strcode;
    union {
        float fval;
        unsigned int uval;
    };

    struct Node* bros;
    struct Node* kids;

} Node;

Node* CreateNode(int t, int l, char *lex, char *s);

void BuildTree(Node* r, int cnt, ...);

void PrintTree(Node* r, int prefix);

extern Node* root;
#endif
