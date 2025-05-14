#include "mipscode.h"

MipsNode AddOneMipsCode(MipsNode mc, char* s, ...) {
    assert(mc != NULL);
    assert(mc->tail == NULL);
   
    va_list args, cp;
    va_start(args, s);
    va_copy(cp, args);
    int len = vsnprintf(NULL, 0, s, cp);
    va_end(cp);
   
    if(len < 0) {va_end(args); return mc;}
   
    MipsNode p = malloc(sizeof(struct MipsNode_));
    if (!p) { perror("malloc"); exit(1); }
    p->code = malloc(len + 1);
    if (!p->code) { perror("malloc"); exit(1); }
    p->tail = NULL;

    vsnprintf(p->code, len + 1, s, args);
    va_end(args);

    mc->tail = p;
    return p; 
}

SpNode offlist = NULL;
SpNode OffHash[16384];

void PrepareHash(void ) {
    SpNode c = offlist;
    while (c != NULL) {
      SpNode p = c->dele;
      free(c->str); free(c); 
      c = p;
    }

    offlist = NULL;
    for (int i = 0; i < 16384; i++) 
    OffHash[i] = NULL;  
}

int PrepareStack(InterCode* ic, MipsNode mc, int spoff) {
    int sp = spoff;
    InterCode* p = ic->next;
   
    while (p != NULL  && p->kind != IC_FUNCTION) {
       if (p->kind == IC_DEC) 
          sp = allocateSp(p->dest, p->size >> 2, 0, sp);
       else {
          if (p->dest != NULL && p->dest[0] != '#') 
          sp = allocateSp(p->dest, 1, 1, sp);
          if(p->arg1 != NULL && p->arg1[0] != '#') 
          sp = allocateSp(p->arg1, 1, 1, sp);
          if(p->arg2 != NULL && p->arg2[0] != '#') 
          sp = allocateSp(p->arg2, 1, 1, sp);
       }
       p = p->next;
    }
    
    mc = AddOneMipsCode(mc, "  addi $sp, $sp, -4");
    mc = AddOneMipsCode(mc, "  sw $s8, 0($sp)");
    mc = AddOneMipsCode(mc, "  la $s8, 4($sp)");
    mc = AddOneMipsCode(mc, "  addi $sp, $sp, -%d", sp << 2);
    
    return sp;
}

int allocateSp(char* name, int slots, int check, int spoff) {
    if (check != 0) {
        SpNode resSp = QuOff(name);
        if (resSp != NULL || (name[0] == '_' && name[1] == 'L'))
            return spoff;  
    }

    int sp = spoff+ slots;
    InOff(name, sp);
    
    return sp;
}

SpNode QuOff(char* s) {
    int hashid = hash_pjw(s);
    SpNode res = OffHash[hashid];
    while (res !=NULL && strcmp(res->str, s) != 0)
      res = res->tail;
    return res;
}

void InOff(char* s, int spoff) {
    int hashid = hash_pjw(s); 
    SpNode res = malloc(sizeof(struct SpNode_));
    res->str = malloc(strlen(s) + 1);
    strcpy(res->str, s); 
    res->cnt = spoff;
    res->tail = OffHash[hashid];
    OffHash[hashid] = res;
    res->dele = offlist;
    offlist = res;
} 

int getOffset(int sp, char* s) {
    SpNode resSp = QuOff(s);
    assert(resSp != NULL);
    return ((sp - resSp->cnt) << 2);
}


