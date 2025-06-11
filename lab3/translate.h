#ifndef TRANSLATE_H
#define TRANSLATE_H

#include "syuhashtable.h"
#include "tree.h"
#include "intercode.h"
#include "syumatin.h"
#include"stdio.h"

InterCodes* translate_ExtDefList(Node* node);
InterCodes* translate_ExtDef(Node* node);
InterCodes* translate_FunDec(Node* node);
InterCodes* translate_VarList(Node* node);
InterCodes* translate_ParamDec(Node* node);
InterCodes* translate_VarDec_function(Node* node);
InterCodes* translate_CompSt(Node* node);
InterCodes* translate_DefList(Node* node);
InterCodes* translate_Def(Node* node);
InterCodes* translate_DecList(Node* node);
InterCodes* translate_Dec(Node* node);
InterCodes* translate_StmtList(Node* node);
InterCodes* translate_Stmt(Node* node);
InterCodes* translate_Exp(Node* node, char* place,int *p);
InterCodes* translate_Cond(Node* node, char* label_true, char* label_false);

typedef struct ArgList ArgList;
typedef struct ArgID ArgID;
struct ArgID {
    char* ID;
    Type type;
    ArgID* next;
};
struct ArgList {
    ArgID* head;
};

InterCodes* translate_Args(Node* node, ArgList* arg_list);

int get_element_size(Type type);
int calculate_stride(Type current_type);
int calculate_offset(Type array_type, int* indices, int dim_index);

#endif
