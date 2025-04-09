#include "syumatin.h"

int deflevel = 0;

void LcEnv(){
    deflevel++;
    AddStackEnv(deflevel);
}

void RcEnv(){
    DelStackEnv(deflevel);
    deflevel--;
}

void do_analysis(Node* r) { Program(r);}

//High-level Definitions 
void Program(Node* r) {
    //→ ExtDefList
    AddStackEnv(deflevel);
    Node* NodeExtDefList = TreeFind(r, "ExtDefList");
    ExtDefList(NodeExtDefList);
    DelStackEnv(deflevel);

}

void ExtDefList(Node* r) {
    if (r == NULL) return ;
    
    //| epsilon 
    if (r->kids == NULL) return ;
    
    //→ ExtDef ExtDefList 
    Node* NodeExtDef = TreeFind(r, "ExtDef");
    ExtDef(NodeExtDef);  
    Node* NodeExtDefList = TreeFind(r, "ExtDefList");
    ExtDefList(NodeExtDefList);
}

void ExtDef(Node* r) {
    if (r == NULL) return ;
    
    Node* NodeSpecifier = TreeFind(r, "Specifier");
    Type ty = Specifier(NodeSpecifier);
    //→ Specifier ExtDecList SEMI 
    Node* NodeExtDecList = TreeFind(r, "ExtDecList");
    if (NodeExtDecList != NULL)
       return ExtDecList(NodeExtDecList, ty);
    //| Specifier SEMI 
    Node* NodeFunDec = TreeFind(r, "FunDec");
    if (NodeFunDec == NULL) return ;
    //| Specifier FunDec CompSt 
    HashNodeFunc func = FunDec(NodeFunDec, ty);

    HashNodeFunc resFunc = QueryFunc(func->name);
    if (resFunc != NULL) 
       printf("Error type 4 at line %d: Redefined function '%s'\n", r->lnumber, func->name);
    else 
       InsertFunc(func);
    //printf("%s,%d\n",func->name,func->retyp->basic);
    Node* NodeCompSt = TreeFind(r, "CompSt");
    CompSt(NodeCompSt, func->retyp);
    RcEnv();
}

void ExtDecList(Node* r, Type ty) {
    if (r == NULL) return ;
    
    //→ VarDec 
    Node* NodeVarDec = TreeFind(r, "VarDec");
    VarDec(NodeVarDec, NULL, ty);
    //| VarDec COMMA ExtDecList 
    Node* NodeExtDecList = TreeFind(r, "ExtDecList");
    ExtDecList(NodeExtDecList, ty);
}

//Specifiers 
Type Specifier(Node* r) {
    if (r == NULL) return NULL;

    //→ TYPE 
    Node* Node_Type = TreeFind(r, "TYPE"); 
    if(Node_Type != NULL) {
       if (strcmp(Node_Type->strcode, "int") == 0) {
          Type p = malloc(sizeof(struct Type_));
          p->kind = BASIC;
          p->basic = 0;
          return p;} 
       else if (strcmp(Node_Type->strcode, "float") == 0) {
          Type p = malloc(sizeof(struct Type_));
          p->kind = BASIC;
          p->basic = 1;
          return p;} 
       assert(0);
    }
    
    //| StructSpecifier
    Node* NodeStructSpecifier = TreeFind(r, "StructSpecifier");
    return StructSpecifier(NodeStructSpecifier);
}

Type StructSpecifier(Node* r) {
    if (r == NULL) return NULL;
    
    //→ STRUCT OptTag LC DefList RC #the def of a Struct
    Node* NodeDefList = TreeFind(r, "DefList");
    if (NodeDefList != NULL) {
       Type newstru = malloc(sizeof(struct Type_));
       newstru->kind = STRUCTURE;
       newstru->structure = malloc(sizeof(struct FieldList_ ));
       newstru->structure->tail = NULL;
       
       LcEnv();
       DefList(NodeDefList, newstru->structure);
       RcEnv();
       /**/
       Node* NodeOptTag = TreeFind(r, "OptTag");
       if (NodeOptTag != NULL) {
          char* str = OptTag(NodeOptTag);
          newstru->structure->name = str;
          
          HashNodeVar resVar = QueryVar(str);
          HashNodeStru resStru = QueryStru(str);
          
          if (resVar != NULL || resStru != NULL) 
             printf("Error type 16 at line %d: Duplicated name '%s'.\n", r->lnumber, str);
          else 
             InsertStru(newstru);
       }
       
       else newstru->structure->name = NULL;
     
       return newstru;
    }
    
    //| STRUCT Tag  #the def of a struct Var
    Node* NodeTag = TreeFind(r, "Tag");
    char* str = Tag(NodeTag);
    HashNodeStru resStru = QueryStru(str);
    if (resStru != NULL) return resStru->type;
     
    printf("Error type 17 at line %d: Undefined structure '%s'.\n", r->lnumber, str);
    return NULL;
}

char* OptTag(Node* r) {
    if (r == NULL) return NULL;//→ ID 
    return TreeFind(r, "ID")->strcode;
}

char* Tag(Node* r) {
    if (r == NULL) return NULL;//→ ID 
    return TreeFind(r, "ID")->strcode;
}

//Declarators 
void VarDec(Node* r, FieldList t, Type ty) {
    if (r == NULL) return ;
    
    // → ID       
    Node* NodeId = TreeFind(r, "ID");
    if (NodeId != NULL) {
      char* str = NodeId->strcode;
      
      HashNodeVar resVar = QueryVar(str);
      HashNodeStru resStru = QueryStru(str);
      
      if (resStru != NULL) 
         printf("Error type 3 at line %d: Redefined variable name '%s'.\n", r->lnumber, str);
      else if (resVar != NULL && resVar->level == deflevel) 
      {
         if (t == NULL) printf("Error type 3 at line %d: Redefined variable name '%s'.\n", r->lnumber, str);
         else printf("Error type 15 at line %d: Redefined field '%s'.\n", r->lnumber, str);
      }
      else 
      {
         if (t != NULL)
         {  
           while(t->tail != NULL) t = t->tail;
           FieldList cur = malloc(sizeof(struct  FieldList_ ));
           cur->name = str;
           cur->type = ty;
           cur->tail = NULL;
           t->tail = cur;
         }
         InsertVar(deflevel, str, ty);
      }
      return ;
    }
    
    //| VarDec LB INT RB 
    Node* NodeInt = TreeFind(r, "INT");
    
    Type arr = malloc(sizeof(struct Type_));
    arr->kind = ARRAY;
    arr->array.size = NodeInt->uval;
    arr->array.elem = ty;
    
    Node* NodeVarDec = TreeFind(r, "VarDec");
    VarDec(NodeVarDec, t, arr);
}

HashNodeFunc FunDec(Node* r, Type ty) {
    if (r == NULL) return NULL;
    
    //| ID LP RP 
    Node* NodeId = TreeFind(r, "ID");
    char* str = NodeId->strcode; 
    HashNodeFunc func = malloc(sizeof(struct HashNodeFunc_ ));
    func->name = str;
    func->retyp = ty;
    func->argvs = malloc(sizeof(struct FieldList_ ));
    func->argvs->tail = NULL;
    func->tail = NULL;
    //→ ID LP VarList RP 
    Node* NodeVarList = TreeFind(r, "VarList");
    LcEnv();
    VarList(NodeVarList, func->argvs);
    
    return func;
}

void VarList(Node* r, FieldList t) {
    if (r == NULL) return ;
    
    //| ParamDec 
    Node* NodeParamDec = TreeFind(r, "ParamDec");
    ParamDec(NodeParamDec, t);
    //→ ParamDec COMMA VarList 
    Node* NodeVarList = TreeFind(r, "VarList");
    VarList(NodeVarList, t);
}

void ParamDec(Node* r, FieldList t) {
    if (r == NULL) return ;
   
    //→ Specifier VarDec 
    Node* NodeSpecifier = TreeFind(r, "Specifier");
    Type ty = Specifier(NodeSpecifier);
    Node* NodeVarDec = TreeFind(r, "VarDec");
    VarDec(NodeVarDec, NULL, ty);
    
    Node* NodeId = NodeVarDec->kids;
    while (NodeId->kids != NULL) NodeId = NodeId->kids; 
    
    while(t->tail != NULL) t = t->tail;
    FieldList cur = malloc(sizeof(struct FieldList_ ));
    cur->name = NodeId->strcode;
    cur->type = ty;
    cur->tail = NULL;
    t->tail = cur;
}

//Statements 
void CompSt(Node* r, Type ret) {
    if (r == NULL) return ;
    
    //→ LC DefList StmtList RC 
    Node* NodeDefList = TreeFind(r, "DefList");
    DefList(NodeDefList, NULL);
    Node* NodeStmtList = TreeFind(r, "StmtList");
    StmtList(NodeStmtList, ret);
}

void StmtList(Node* r, Type ret) {
    if (r == NULL) return ;
    
    //| epsilon 
    if (r->kids == NULL) return ;
    //→ Stmt StmtList 
    Node* NodeStmt = TreeFind(r, "Stmt");
    Stmt(NodeStmt, ret);
    Node* NodeStmtList = TreeFind(r, "StmtList");
    StmtList(NodeStmtList, ret);
}    

void Stmt(Node* r, Type ret) {
    if (r == NULL) return ;
  
    //| CompSt
    Node* NodeCompSt = TreeFind(r, "CompSt");
    if (NodeCompSt != NULL ) {
       LcEnv();
       CompSt(NodeCompSt, ret);
       RcEnv();
       return ;
    }
    //→ Exp SEMI
    Node* NodeExp = TreeFind(r, "Exp");
    Type res = Exp(NodeExp); 
    
    //| RETURN Exp SEMI 
    Node* NodeReturn = TreeFind(r, "RETURN");
    if (NodeReturn != NULL) {
       if (CheckType(res, ret) == 0) 
            printf("Error type 8 at line %d: Type mismatched for return.\n", r->lnumber);
       return;
    }
    
    //| IF LP Exp RP Stmt 
    //| WHILE LP Exp RP Stmt
    Node* NodeStmt = TreeFind(r, "Stmt");
    Stmt(NodeStmt, ret);
    
    //| IF LP Exp RP Stmt ELSE Stmt 
    if (NodeStmt != NULL && NodeStmt->bros != NULL) 
       Stmt(NodeStmt->bros->bros, ret);
    
}    
 
//Local Definitions 
void DefList(Node* r, FieldList t) {
    if (r == NULL) return ;
    if (r->kids == NULL) return;
    //→ Def DefList 
    Node* NodeDef = TreeFind(r, "Def");
    Def(NodeDef, t);
    Node* NodeDefList = TreeFind(r, "DefList");
    DefList(NodeDefList, t);
}
void Def(Node* r, FieldList t) {
    if (r == NULL) return ;
    
    //→ Specifier DecList SEMI 
    Node* NodeSpecifier = TreeFind(r, "Specifier");
    Type ty = Specifier(NodeSpecifier);
    
    Node* NodeDecList = TreeFind(r, "DecList");
    DecList(NodeDecList, t, ty);
}

void DecList(Node* r, FieldList t, Type ty) {
   if (r == NULL) return ;
   //→ Dec
   Node* NodeDec = TreeFind(r, "Dec");
   Dec(NodeDec, t, ty);
   //| Dec COMMA DecList 
   Node* NodeDecList = TreeFind(r, "DecList");
   DecList(NodeDecList, t, ty);
}

void Dec(Node* r, FieldList t, Type ty) {
    if (r == NULL) return;
    
    //→ VarDec 
    Node* NodeVarDec = TreeFind(r, "VarDec");
    //| VarDec ASSIGNOP Exp 
    Node* NodeExp = TreeFind(r, "Exp");
    
    if (t != NULL && NodeExp != NULL) {
       printf("Error type 15 at line %d: Assignment in struct definition.\n", r->lnumber);
       return ; 
    }

    VarDec(NodeVarDec, t, ty);
    
    if (NodeExp != NULL) 
       if ( CheckType(ty, Exp(NodeExp)) == 0 ) 
         printf("Error type 5 at line %d: Type mismatched for assignment.\n", r->lnumber);    
}

//Expressions 
Type Exp(Node* r) {
    if (r == NULL) return NULL;
    
    //| Exp ASSIGNOP Exp
    Node* NodeAssignop = TreeFind(r, "ASSIGNOP");
    if (NodeAssignop != NULL) {
      Type ty1 = Exp(r->kids);
      Type ty2 = Exp(NodeAssignop->bros);
      if (CheckType(ty1, ty2) == 0) 
         printf("Error type 5 at line %d: Type mismatched for assignment.\n", r->lnumber);
      
      if (CheckLeft(r->kids) == 0) 
         printf("Error type 6 at line %d: The left-hand side of an assignment must be a variable.\n", r->lnumber);
      return ty1;
    }
   
    //| Exp AND Exp only int
    //| Exp OR Exp  
    Node* NodeAndOr = TreeFind(r, "AND");
    NodeAndOr = (NodeAndOr == NULL)?TreeFind(r, "OR"):NodeAndOr;
    if (NodeAndOr != NULL) {
       Type ty1 = Exp(r->kids);
       Type ty2 = Exp(NodeAndOr->bros);
       if (ty1->kind != BASIC || ty1->basic != 0) 
          printf("Error type 7 at line %d: Type mismatched for operands.\n", r->lnumber);
       else if (ty2->kind != BASIC || ty2->basic != 0) 
          printf("Error type 7 at line %d: Type mismatched for operands.\n", r->lnumber);
       return ty1;
    }
    
    //| Exp RELOP Exp
    Node* NodeRelop = TreeFind(r, "RELOP");
    if (NodeRelop != NULL) {
      Type ty1 = Exp(r->kids);
      Type ty2 = Exp(NodeRelop->bros);
      if (CheckType(ty1, ty2) == 0) 
         printf("Error type 7 at line %d: Type mismatched for operands.\n", r->lnumber);
      else if (ty1->kind != BASIC) 
         printf("Error type 7 at line %d: Type mismatched for operands.\n", r->lnumber);   
       
      Type p = malloc(sizeof(struct Type_));
      p->kind = BASIC;
      p->basic = 0;
      return p;
    }

    //| MINUS Exp 
    //| NOT Exp 
    if (r->kids->bros != NULL && r->kids->bros->bros == NULL) {
       Type ty1 = Exp(r->kids->bros);
       if (ty1->kind != BASIC) 
         printf("Error type 7 at line %d: Type mismatched for operands.\n", r->lnumber);
       else if (TreeFind(r, "NOT") != NULL) {
         if (ty1->basic != 0) {
           printf("Error type 7 at line %d: Type mismatched for operands.\n", r->lnumber);
           Type p = malloc(sizeof(struct Type_));
           p->kind = BASIC;
           p->basic = 0;
           return p;}
       }
       return ty1;
    }

    //| LP Exp RP 
    Node* NodeLp = TreeFind(r, "LP");
    if (NodeLp == r->kids) {
       Type ty1 = Exp(NodeLp->bros);
       return ty1;
    } 
    //| ID LP Args RP  #Function
    //| ID LP RP
    else if (NodeLp != NULL) {
       Node* NodeId = r->kids;
       HashNodeFunc resFunc = QueryFunc(NodeId->strcode);
       if (resFunc != NULL) {
         Node* NodeArgs = TreeFind(r, "Args");
         Args(NodeArgs, resFunc->argvs->tail, resFunc->name);
         return resFunc->retyp;}
       HashNodeVar resVar = QueryVar(NodeId->strcode);
       if (resVar == NULL)
         printf("Error type 2 at line %d: Undefined function '%s'.\n", r->lnumber, NodeId->strcode);
       else  printf("Error type 11 at Line %d: '%s' is not a function.\n", r->lnumber, NodeId->strcode);
       return NULL;
    }

    //| ID 
    Node* NodeId = TreeFind(r, "ID");
    if (NodeId == r->kids) {
      
       HashNodeVar resVar = QueryVar(NodeId->strcode);
       if (resVar == NULL)
         {printf("Error type 1 at line %d: Undefined variable '%s'.\n", r->lnumber, NodeId->strcode); return NULL;}
       return resVar->type;
    }    
    //| Exp DOT ID  #Sturct 
    else if (NodeId != NULL) {
       Type ty1 = Exp(r->kids);
       if (ty1->kind != STRUCTURE) 
          {printf("Error type 13 at line %d: Illegal use of dot.\n", r->lnumber); return NULL;}
       FieldList t = ty1->structure->tail;
       while (t != NULL) {
         if (strcmp(t->name, NodeId->strcode) == 0) return t->type;
         t = t->tail; 
       }
       {printf("Error type 14 at line %d: Non-existent '%s'.\n", r->lnumber, NodeId->strcode); return NULL;}
    }
    
    //| Exp LB Exp RB  #Array 
    Node* NodeLb = TreeFind(r, "LB");
    if (NodeLb != NULL) {
       Type ty1 = Exp(r->kids);
       Type ty2 = Exp(NodeLb->bros);
       
       if (ty1->kind != ARRAY) 
          {printf("Error type 10 at line %d: Not an array.\n", r->lnumber); return NULL;}
       if (ty2->kind != BASIC || ty2->basic !=0) 
          {printf("Error type 12 at line %d: Not an integer.\n", r->lnumber); return NULL;}
       return ty1->array.elem;
    }
   
    //| INT 
    Node* NodeInt = TreeFind(r, "INT");
    if (NodeInt != NULL) {
       Type p = malloc(sizeof(struct Type_));
       p->kind = BASIC;
       p->basic = 0;
       return p;
    }  
    //| FLOAT 
    Node* NodeIntf = TreeFind(r, "FLOAT");
    if (NodeIntf != NULL) {
       Type p = malloc(sizeof(struct Type_));
       p->kind = BASIC;
       p->basic = 1;
       return p;
    }
                
    //| Exp PLUS Exp 
    //| Exp MINUS Exp 
    //| Exp STAR Exp 
    //| Exp DIV Exp 
    Type ty1 = Exp(r->kids);
    Type ty2 = Exp(r->kids->bros->bros);
    if (CheckType(ty1, ty2) == 0) 
       printf("Error type 7 at line %d: Type mismatched for operands.\n", r->lnumber);
    else if (ty1->kind != BASIC) 
       printf("Error type 7 at line %d: Type mismatched for operands.\n", r->lnumber);   
    return ty1;
}

void Args(Node* r, FieldList arg, char* name) {
    if (r == NULL && arg == NULL ) return;
    else if (r == NULL || arg == NULL )
      {printf("Error type 9 at line %d: Args mismatch to '%s'.\n", r->lnumber, name); return ;}

    //| Exp 
    Node* NodeExp = TreeFind(r, "Exp");
    Type res = Exp(NodeExp); 
    if (CheckType(res, arg->type) == 0)
      printf("Error type 9 at line %d: Args mismatch to '%s'.\n", r->lnumber, name);
    //→ Exp COMMA Args 
    Node* NodeArgs = TreeFind(r, "Args");
    Args(NodeArgs, arg->tail, name);
}

Node* TreeFind(Node* r, char* name) {
    if (r == NULL ) return NULL;
    
    Node* p = r->kids;
    while(p != NULL) {
        if (strcmp(p->lextype, name) == 0) break;
        p = p->bros;
    }
    return p;
}

int CheckLeft(Node* exp) {
    if (exp == NULL) return 0;

    // Exp → Exp DOT ID
    Node* NodeDot = TreeFind(exp, "DOT");
    if (NodeDot != NULL) {
       Type ty = Exp(exp->kids);
       if (ty == NULL || ty->kind != STRUCTURE)
           return 0;
        
       Node* NodeId = TreeFind(exp, "ID");
                       
       FieldList t = ty->structure->tail;
        while (t != NULL) {
            if (strcmp(t->name, NodeId->strcode) == 0) 
              return 1; 
            t = t->tail;
        }
        return 0; 
    }
    
    // Exp → ID
    Node* NodeId = TreeFind(exp, "ID");
    if (NodeId != NULL) {
       if (exp->kids == NodeId && NodeId->bros == NULL) 
         return 1;
       else return 0;
    }
    // Exp → Exp LB Exp RB
    Node* NodeLb = TreeFind(exp, "LB");
    if (NodeLb != NULL) {
       Type ty = Exp(exp->kids);
       if (ty != NULL || ty->kind == ARRAY) 
          return 1;
       return 0; 
    }
    return 0; // else
}


