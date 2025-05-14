#ifndef MIPSCODE_H
#define MIPSCODE_H

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "intercode.h"

typedef struct MipsNode_*  MipsNode;
typedef struct SpNode_* SpNode;

struct MipsNode_ {
    char* code;
    MipsNode tail;
};

MipsNode AddOneMipsCode(MipsNode mc, char* s, ...); 

struct SpNode_ {
    char* str;
    int cnt;
    SpNode tail;
    SpNode dele;
};

void PrepareHash(void );
int PrepareStack(InterCode* ic, MipsNode mc, int spoff);
int allocateSp(char* name, int slots, int check, int spoff);
SpNode QuOff(char* s);
void InOff(char* s, int spoff);
int getOffset(int sp, char* s); 

#endif
