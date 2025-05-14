#include "object.h"

static int spoff = 0;
static int argcnt = 0;
static int stackcnt = 0;
static int paramcnt = 0;

//tanslate
MipsNode translate_ic(InterCodes* ics) {
    MipsNode mchead = malloc(sizeof(struct MipsNode_));
    mchead->tail = NULL; mchead->code = NULL;
    MipsNode mctail = PrepareMipsEnv(mchead);
    
    InterCode* ic = ics->head;
    while(ic != NULL) {
        mctail = trans_code(ic, mctail);
        ic = ic->next;
    }
    return mchead;
}

MipsNode PrepareMipsEnv(MipsNode mc) {
    MipsNode p = mc;
    p = AddOneMipsCode(p, ".data");
    p = AddOneMipsCode(p, "_prompt: .asciiz \"Enter an integer:\"");
    p = AddOneMipsCode(p, "_ret: .asciiz \"\\n\"");
    p = AddOneMipsCode(p, ".globl main");
    p = AddOneMipsCode(p, ".text");
    p = AddOneMipsCode(p, "read:");
    p = AddOneMipsCode(p, "  li $v0, 4");
    p = AddOneMipsCode(p, "  la $a0, _prompt");
    p = AddOneMipsCode(p, "  syscall");
    p = AddOneMipsCode(p, "  li $v0, 5");
    p = AddOneMipsCode(p, "  syscall");
    p = AddOneMipsCode(p, "  jr $ra");
    p = AddOneMipsCode(p, "  ");
    p = AddOneMipsCode(p, "write:");
    p = AddOneMipsCode(p, "  li $v0, 1");
    p = AddOneMipsCode(p, "  syscall");
    p = AddOneMipsCode(p, "  li $v0, 4");
    p = AddOneMipsCode(p, "  la $a0, _ret");
    p = AddOneMipsCode(p, "  syscall");
    p = AddOneMipsCode(p, "  move $v0, $0");
    p = AddOneMipsCode(p, "  jr $ra");
    
    assert(p != NULL && p->tail == NULL && strstr(p->code, "jr $ra") != NULL);
    p = AddOneMipsCode(p, "  ");
    return p;
}

MipsNode move_to_tail(MipsNode p) {
    while (p->tail != NULL)
     p = p->tail;
    return p;
}

MipsNode trans_code(InterCode* ic, MipsNode mcnode) {
   
    assert(mcnode != NULL);
    assert(mcnode->tail == NULL);    
    
    switch(ic->kind) {
    case IC_LABEL: 
        tansForLABEL(ic, mcnode); break;
    case IC_FUNCTION: 
       tansForFUNCTION(ic, mcnode); break;
    case IC_ASSIGN: 
        tansForASSIGN(ic, mcnode); break;
    case IC_PLUS: 
        tansForC2(ic, mcnode); break;
    case IC_MINUS: 
        tansForC2(ic, mcnode); break;
    case IC_MUL: 
        tansForC2(ic, mcnode); break;
    case IC_DIV: 
        tansForC2(ic, mcnode); break;
    case IC_ADDROF: 
        tansForADDROF(ic, mcnode); break;
    case IC_ASSIGNFROMADDR: 
        tansForASSIGNFROMADDR(ic, mcnode); break;
    case IC_ASSIGNTOADDR: 
        tansForASSIGNTOADDR(ic, mcnode); break;
    case IC_GOTO: 
        tansForGOTO(ic, mcnode); break;
    case IC_IFGOTO: 
        tansForIFGOTO(ic, mcnode); break;
    case IC_RETURN: 
        tansForRETURN(ic, mcnode); break;
    case IC_DEC: 
        tansForDEC(ic, mcnode); break;
    case IC_ARG: 
        tansForARG(ic, mcnode); break;
    case IC_ASSIGNCALL: 
        tansForASSIGNCALL(ic, mcnode); break;
    case IC_PARAM: 
        tansForPARAM(ic, mcnode); break;
    case IC_READ: 
        tansForREAD(ic, mcnode); break;
    case IC_WRITE: 
        tansForWRITE(ic, mcnode); break;
    default:
        fprintf(stderr, "Unrecognized IC kind: %d\n", ic->kind);
        exit(0);
    }
    
    return move_to_tail(mcnode);
}

void tansForLABEL(InterCode* ic, MipsNode mc) {
    mc = AddOneMipsCode(mc, "%s:", ic->dest);
}
void tansForFUNCTION(InterCode* ic, MipsNode mc) {
    static const char* current = NULL;
    if (current != NULL && strcmp(current, ic->dest) == 0) return ; 
    current = ic->dest;

    mc = AddOneMipsCode(mc, "%s:", ic->dest);
    spoff = argcnt = paramcnt = 0;
    PrepareHash();
    spoff = PrepareStack(ic, mc, spoff);
}
void tansForASSIGN(InterCode* ic, MipsNode mc) {
    if(ic->arg1[0] == '#') 
    mc = AddOneMipsCode(mc, "  li $t0, %s", ic->arg1 + 1);     
    else { int of = getOffset(spoff, ic->arg1);
    mc = AddOneMipsCode(mc, "  lw $t0, %d($sp)", of);}
    int of = getOffset(spoff, ic->dest);
    mc = AddOneMipsCode(mc, "  sw $t0, %d($sp)", of);
}
void tansForC2(InterCode* ic, MipsNode mc) {
    if(ic->arg1[0] == '#') 
    mc = AddOneMipsCode(mc, "  li $t0, %s", ic->arg1 + 1);     
    else { int of = getOffset(spoff, ic->arg1);
    mc = AddOneMipsCode(mc, "  lw $t0, %d($sp)", of);}
    
    if(ic->arg2[0] == '#') 
    mc = AddOneMipsCode(mc, "  li $t1, %s", ic->arg2 + 1);     
    else { int of = getOffset(spoff, ic->arg2);
    mc = AddOneMipsCode(mc, "  lw $t1, %d($sp)", of);}

    switch (ic->kind) {
        case IC_PLUS:  mc = AddOneMipsCode(mc, "  add $t0, $t0, $t1"); break;
        case IC_MINUS: mc = AddOneMipsCode(mc, "  sub $t0, $t0, $t1");  break;
        case IC_MUL:  mc = AddOneMipsCode(mc, "  mul $t0, $t0, $t1");  break;
        case IC_DIV:  mc = AddOneMipsCode(mc, "  div $t0, $t1");
            mc = AddOneMipsCode(mc, "  mflo $t0"); break;
        default:
        fprintf(stderr, "Unexcepted OP kind: %d\n", ic->kind);
        exit(0);
    }
    
    int of = getOffset(spoff, ic->dest);
    mc = AddOneMipsCode(mc, "  sw $t0, %d($sp)", of);
}
void tansForADDROF(InterCode* ic, MipsNode mc) {
    mc = AddOneMipsCode(mc, "  addiu $t0, $sp, %d", getOffset(spoff, ic->arg1));               
    mc = AddOneMipsCode(mc, "  sw $t0, %d($sp)", getOffset(spoff, ic->dest));           
}
void tansForASSIGNFROMADDR(InterCode* ic, MipsNode mc) {
    mc = AddOneMipsCode(mc, "  lw $t0, %d($sp)", getOffset(spoff, ic->arg1));   
    mc = AddOneMipsCode(mc, "  lw $t0, 0($t0)");          
    mc = AddOneMipsCode(mc, "  sw $t0, %d($sp)", getOffset(spoff, ic->dest));    
}
void tansForASSIGNTOADDR(InterCode* ic, MipsNode mc) {
    mc = AddOneMipsCode(mc, "  lw $t0, %d($sp)", getOffset(spoff, ic->dest));   
    mc = AddOneMipsCode(mc, "  lw $t1, %d($sp)", getOffset(spoff, ic->arg1));   
    mc = AddOneMipsCode(mc, "  sw $t1, 0($t0)");           
}
void tansForGOTO(InterCode* ic, MipsNode mc){
    mc = AddOneMipsCode(mc, "  j %s", ic->dest);
}
void tansForIFGOTO(InterCode* ic, MipsNode mc) {
    if(ic->arg1[0] == '#') 
    mc = AddOneMipsCode(mc, "  li $t0, %s", ic->arg1 + 1);     
    else { int of = getOffset(spoff, ic->arg1);
    mc = AddOneMipsCode(mc, "  lw $t0, %d($sp)", of);}
    
    if(ic->arg2[0] == '#') 
    mc = AddOneMipsCode(mc, "  li $t1, %s", ic->arg2 + 1);     
    else { int of = getOffset(spoff, ic->arg2);
    mc = AddOneMipsCode(mc, "  lw $t1, %d($sp)", of);}
    
    char* instr = NULL;
    switch (ic->relop->kind) {
    case REL_EQ: instr = "beq"; break;   /* == */
    case REL_NEQ: instr = "bne"; break;   /* != */
    case REL_GT: instr = "bgt"; break;   /* >  */
    case REL_LT: instr = "blt"; break;   /* <  */
    case REL_GEQ: instr = "bge"; break;   /* >= */
    case REL_LEQ: instr = "ble"; break;   /* <= */
    case REL_NEG: instr = NULL; break;  /* !x  →  beq x,$0,label */
    default:
        fprintf(stderr, "Illegal relop kind %d in IFGOTO.\n", ic->relop->kind);
        exit(1);
    }  
    if (ic->relop->kind == REL_NEG)
    mc = AddOneMipsCode(mc, "  beq $t0, $0, %s", ic->dest);
    else mc = AddOneMipsCode(mc, "  %s $t0, $t1, %s", instr, ic->dest);
}
void tansForRETURN(InterCode* ic, MipsNode mc) {
    int of = getOffset(spoff, ic->dest);
    mc = AddOneMipsCode(mc, "  lw $t0, %d($sp)", of);
    mc = AddOneMipsCode(mc,"  move $v0, $t0");
    mc = AddOneMipsCode(mc, "  move $sp, $s8");
    mc = AddOneMipsCode(mc, "  lw $s8, -4($s8)");
    mc = AddOneMipsCode(mc, "  jr $ra");
}
void tansForDEC(InterCode* ic, MipsNode mc) {
    return ;
}
void tansForARG(InterCode* ic, MipsNode mc) {
    int of = getOffset(spoff, ic->dest);
    mc = AddOneMipsCode(mc, "  lw $t0, %d($sp)", of + (argcnt << 2) );
    mc = AddOneMipsCode(mc, "  addi $sp, $sp, -4");  
    mc = AddOneMipsCode(mc, "  sw $t0, 0($sp)"); 
    argcnt = argcnt + 1;
}
void tansForASSIGNCALL(InterCode* ic, MipsNode mc) {
    for (int i = 0; i < 4 && i < argcnt; i++)        
        mc = AddOneMipsCode(mc, "  lw $a%d, %d($sp)", i, i << 2);    

    mc = AddOneMipsCode(mc,"  addi $sp, $sp, -4");
    mc = AddOneMipsCode(mc,"  sw $ra, 0($sp)");
    mc = AddOneMipsCode(mc,"  jal %s", ic->arg1);
    mc = AddOneMipsCode(mc,"  lw $ra, 0($sp)");
    mc = AddOneMipsCode(mc,"  addi $sp, $sp, 4");
    if (argcnt > 0)
        mc = AddOneMipsCode(mc, "  addi $sp, $sp, %d", argcnt << 2);
        
    mc = AddOneMipsCode(mc, "  move $t0, $v0");
    int of = getOffset(spoff, ic->dest);
    mc = AddOneMipsCode(mc, "  sw $t0, %d($sp)", of);
    argcnt = 0;
}
void tansForPARAM(InterCode* ic, MipsNode mc) {
    paramcnt = paramcnt + 1;
    if(paramcnt < 5) mc = AddOneMipsCode(mc, "  move $t0, $a%d", paramcnt - 1);
    else mc = AddOneMipsCode(mc, "  lw $t0, %d($s8)", (paramcnt - 4) << 2);
    
    int of = getOffset(spoff, ic->dest);
    mc = AddOneMipsCode(mc, "  sw $t0, %d($sp)", of);
}
void tansForREAD(InterCode* ic, MipsNode mc) {
    int of = getOffset(spoff, ic->dest);
    mc = AddOneMipsCode(mc, "  addi $sp, $sp, -4");
    mc = AddOneMipsCode(mc, "  sw $ra, 0($sp)");
    mc = AddOneMipsCode(mc, "  jal read");
    mc = AddOneMipsCode(mc, "  lw $ra, 0($sp)");
    mc = AddOneMipsCode(mc, "  addi $sp, $sp, 4");
    mc = AddOneMipsCode(mc, "  move $t0, $v0");
    mc = AddOneMipsCode(mc, "  sw $t0, %d($sp)", of);
}
void tansForWRITE(InterCode* ic, MipsNode mc) {
    int of = getOffset(spoff, ic->dest);
    mc = AddOneMipsCode(mc, "  lw $t0, %d($sp)", of);
    mc = AddOneMipsCode(mc, "  move $a0, $t0");  
    mc = AddOneMipsCode(mc, "  addi $sp, $sp, -4");
    mc = AddOneMipsCode(mc, "  sw $ra, 0($sp)");
    mc = AddOneMipsCode(mc, "  jal write");
    mc = AddOneMipsCode(mc, "  lw $ra, 0($sp)");
    mc = AddOneMipsCode(mc, "  addi $sp, $sp, 4");
    mc = AddOneMipsCode(mc, "  move $t0, $v0");  
}

// output
void fprint_MipsCodes(FILE* f, MipsNode mchead) {
    MipsNode mc = mchead->tail;
    while(mc != NULL) {
        fprintf(f, "%s\n", mc->code);
        mc = mc->tail;
    }
}
