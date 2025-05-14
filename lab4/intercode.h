#ifndef INTERCODE_H
#define INTERCODE_H

#include <string.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "syuhashtable.h"
//Codes链表，Code单条指令
typedef struct InterCodes InterCodes;
typedef struct InterCode InterCode;
typedef struct InterRelop InterRelop;

struct InterCodes {
    InterCode* head, *tail;
};

struct InterCode {
    enum {
        IC_LABEL, IC_FUNCTION,
        IC_ASSIGN, IC_PLUS, IC_MINUS, IC_MUL, IC_DIV, IC_ADDROF, IC_ASSIGNFROMADDR, IC_ASSIGNTOADDR,
        IC_GOTO, IC_IFGOTO,
        IC_RETURN, IC_DEC, IC_ARG, IC_ASSIGNCALL, IC_PARAM,
        IC_READ, IC_WRITE
    } kind;
    char* arg1, *arg2, *dest;
    // For IC_FUNCTION, dest <- f
    // For IC_IFGOTO, arg1 <- x, arg2 <- y, dest <- z
    // For IC_ASSIGNCALL, dest <- x, arg1 <- f
    // For other InterCodes, dest <- x, arg1 <- y and arg2 <- z
    int size; // For IC_DEC
    InterRelop* relop; // For IC_IFGOTO

    InterCode* next, *prev;//双向链表
};

struct InterRelop {
    enum {
        REL_LT, REL_GT, REL_LEQ, REL_GEQ, REL_EQ, REL_NEQ, REL_NEG
    } kind;
};

// 参数列表
bool isregisteredParam(char* name);
void registerParam(char* name);

// utils
int floor_log10(int k);

// new
char* new_temp();
char* new_label();

// InterCode
InterCodes* get_empty_InterCodes();//创建空链表容器
InterCodes* InterCode2InterCodes(InterCode* code);//将单条指令包装为链表
InterCode* new_InterCode(int kind, ...);
void add_InterCode(InterCodes* target, InterCode* cur);//将新指令cur连接到链表尾部
void add_InterCodes(InterCodes* target, InterCodes* cur);//将cur链表整体接入target尾部

// Output
void fprint_InterCode(FILE* f, InterCode* code);//向f中输出单条指令
void fprint_InterCodes(FILE* f, InterCodes* codes);//按链表顺序向文件f中逐个输出指令
void fprint_InterRelop(FILE* f, InterRelop* relop);

#endif
