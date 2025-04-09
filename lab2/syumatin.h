#ifndef SYUMATIN_H
#define SYUMATIN_H
#include "syuhashtable.h"
#include "tree.h"

Node* TreeFind(Node* r, char* name) ;

int CheckLeft(Node* exp) ;

void do_analysis(Node* r) ;
//High-level Definitions 
void Program(Node* r);
void ExtDefList(Node* r);
void ExtDef(Node* r);
void ExtDecList(Node* r, Type ty);
//Specifiers 
Type Specifier(Node* r);
Type StructSpecifier(Node* r);
char* OptTag(Node* r);
char* Tag(Node* r);
//Declarators 
void VarDec(Node* r, FieldList t, Type ty);
HashNodeFunc FunDec(Node* r, Type ty);
void VarList(Node* r, FieldList t);
void ParamDec(Node* r, FieldList t);
//Statements 
void CompSt(Node* r, Type ret);
void StmtList(Node* r, Type ret);
void Stmt(Node* r, Type ret);
//Local Definitions 
void DefList(Node* r, FieldList t);
void Def(Node* r, FieldList t);
void DecList(Node* r, FieldList t, Type ty);
void Dec(Node* r, FieldList t, Type ty);
//Expressions 
Type Exp(Node* r);
void Args(Node* r, FieldList arg, char* name);

#endif
