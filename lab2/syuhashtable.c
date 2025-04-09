#include "syuhashtable.h"

unsigned int hash_pjw(char* name) 
{ 
    unsigned int val = 0, i; 
    for (; *name; ++name) 
    { 
      val = (val << 2) + *name; 
      if (i = val & ~0x3fff) val = (val ^ (i >> 12)) & 0x3fff; 
    } 
    return val; 
} 

HashNodeVar VarStack = NULL;
HashNodeVar VarTable[16384];
HashNodeStru StruTable[16384];
HashNodeFunc FuncTable[16384];
 
void AddStackEnv(int dept) {
    HashNodeVar env = malloc(sizeof(struct HashNodeVar_));
    env->level = dept;
    env->name = NULL;
    env->table = VarStack;
    env->stcak = NULL;
    VarStack = env;
}

void DelStackList(HashNodeVar env) {
    if (env == NULL) return ;
    int hashid = hash_pjw(env->name);
    VarTable[hashid] = env->table;
    DelStackList(env->stcak);
    free(env);
}

void DelStackEnv(int dept) {
    if (VarStack->level == dept) {
       HashNodeVar env = VarStack;
       VarStack = env->table;
       DelStackList(env->stcak);
       free(env);
    }
}

int CheckType(Type x, Type y) {
    if (x == NULL || y == NULL) return 0;
    if (x->kind != y->kind) return 0;
    
    if (x->kind == BASIC) { if (x->basic != y->basic) return 0;}
    else if (x->kind == ARRAY) return CheckType(x->array.elem, y->array.elem);
    else if (x->kind == STRUCTURE) {
       if (x->structure->name == NULL || y->structure->name == NULL)
         {if (x->structure->name != y->structure->name ) return 0;}
       else if (strcmp(x->structure->name, y->structure->name) != 0) return 0;
    }

    return 1;
}

HashNodeVar QueryVar(char* s) {
    int hashid = hash_pjw(s);
    HashNodeVar res = VarTable[hashid];
    while(res !=NULL && strcmp(res->name, s) != 0)
      res = res->table;
    return res;
}

HashNodeStru QueryStru(char* s) {
    int hashid = hash_pjw(s);
    HashNodeStru res = StruTable[hashid];
    while(res !=NULL && strcmp(res->type->structure->name, s) != 0)
      res = res->tail;
    return res;
}

HashNodeFunc QueryFunc(char* s) {
    int hashid = hash_pjw(s);
    HashNodeFunc res = FuncTable[hashid];
    while(res !=NULL && strcmp(res->name, s) != 0)
      res = res->tail;
    return res;
}

void InsertVar(int l, char* s, Type ty) {
    int hashid = hash_pjw(s); 
    HashNodeVar n = malloc(sizeof(struct HashNodeVar_));
    n->level = l;
    n->name = s;
    n->type = ty;
    n->table = VarTable[hashid];
    VarTable[hashid] = n;
    assert(l == VarStack->level);
    n->stcak = VarStack->stcak;
    VarStack->stcak = n;
}

void InsertStru(Type ty) {
    int hashid = hash_pjw(ty->structure->name); 
    HashNodeStru n = malloc(sizeof(struct HashNodeStru_));
    n->type = ty; 
    n->tail = StruTable[hashid];
    StruTable[hashid] = n;
}

void InsertFunc(HashNodeFunc func) {
    int hashid = hash_pjw(func->name); 
    func->tail = FuncTable[hashid];
    FuncTable[hashid] = func;
}


