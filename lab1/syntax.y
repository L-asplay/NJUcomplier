%{
    #include "manba.h"
    #include "lex.yy.c"
    char emp[1] = "\0";
    extern int syntax_bool;
    int yylex();
    int yyerror(char *);
%}

%union {
    Node* nodetype;
}

%locations

%token <nodetype> FLOAT INT DOT COMMA SEMI ASSIGNOP NOT RELOP PLUS MINUS STAR DIV AND OR LP RP LB RB LC RC TYPE IF ELSE WHILE RETURN STRUCT ID LCOM

%type <nodetype> Program ExtDefList ExtDef ExtDecList Specifier StructSpecifier OptTag Tag VarDec FunDec VarList ParamDec CompSt StmtList Stmt DefList Def DecList Dec Exp Args

%right ASSIGNOP
%left OR AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LP RP LB RB DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

Program : ExtDefList { $$ = CreateNode(0, @$.first_line, "Program", emp); BuildTree($$, 1, $1); root = $$; }
  ;

ExtDefList : ExtDef ExtDefList { $$ = CreateNode(0, @$.first_line, "ExtDefList", emp); BuildTree($$, 2, $1, $2); }
  | /* epsilon */ { $$ = NULL; }
  ;

ExtDef : Specifier ExtDecList SEMI { $$ = CreateNode(0, @$.first_line, "ExtDef", emp); BuildTree($$, 3, $1, $2, $3); }
  | Specifier SEMI { $$ = CreateNode(0, @$.first_line, "ExtDef", emp); BuildTree($$, 2, $1, $2); }
  | Specifier FunDec CompSt { $$ = CreateNode(0, @$.first_line, "ExtDef", emp); BuildTree($$, 3, $1, $2, $3); }
  | error SEMI { $$ = NULL; syntax_bool = 1; }
  | Specifier error { $$ = NULL; syntax_bool = 1; }
  | Specifier FunDec error { $$ = NULL; syntax_bool = 1; }
  ;

ExtDecList : VarDec { $$ = CreateNode(0, @$.first_line, "ExtDecList", emp); BuildTree($$, 1, $1); }
  | VarDec COMMA ExtDecList { $$ = CreateNode(0, @$.first_line, "ExtDecList", emp); BuildTree($$, 3, $1, $2, $3); }
  | error COMMA { $$ = NULL; syntax_bool = 1; }
  ;
  
 
Specifier : TYPE { $$ = CreateNode(0, @$.first_line, "Specifier", emp); BuildTree($$, 1, $1); }
  | StructSpecifier { $$ = CreateNode(0, @$.first_line, "Specifier", emp); BuildTree($$, 1, $1); }
  ;
  
StructSpecifier : STRUCT OptTag LC DefList RC { $$ = CreateNode(0, @$.first_line, "StructSpecifier", emp); BuildTree($$, 5, $1, $2, $3, $4, $5); }
  | STRUCT Tag { $$ = CreateNode(0, @$.first_line, "StructSpecifier", emp); BuildTree($$, 2, $1, $2); }
  |  STRUCT error RC { $$ = NULL; syntax_bool = 1; }
  ;
  
OptTag : ID { $$ = CreateNode(0, @$.first_line, "OptTag", emp); BuildTree($$, 1, $1); } 
  | /* epsilon */ { $$ = NULL; }
  ; 
  
Tag : ID { $$ = CreateNode(0, @$.first_line, "Tag", emp); BuildTree($$, 1, $1); } 
  ;
  
  
VarDec : ID { $$ = CreateNode(0, @$.first_line, "VarDec", emp); BuildTree($$, 1, $1); } 
  | VarDec LB INT RB { $$ = CreateNode(0, @$.first_line, "VarDec", emp); BuildTree($$, 4, $1, $2, $3, $4); }
  | error RB { $$ = NULL; syntax_bool = 1; }
  | error LB INT RB { $$ = NULL; syntax_bool = 1; }
  | VarDec LB error RB { $$ = NULL; syntax_bool = 1; }
  ;

FunDec : ID LP VarList RP { $$ = CreateNode(0, @$.first_line, "FunDec", emp); BuildTree($$, 4, $1, $2, $3, $4); }
  | ID LP RP { $$ = CreateNode(0, @$.first_line, "FunDec", emp); BuildTree($$, 3, $1, $2, $3); }
  | ID LP error RP { $$ = NULL; syntax_bool = 1; }
  | ID error RP { $$ = NULL; syntax_bool = 1; }
  ;

VarList : ParamDec COMMA VarList { $$ = CreateNode(0, @$.first_line, "VarList", emp); BuildTree($$, 3, $1, $2, $3); }
  | ParamDec { $$ = CreateNode(0, @$.first_line, "VarList", emp); BuildTree($$, 1, $1); }
  | error COMMA VarList { $$ = NULL; syntax_bool = 1; }
  ;

ParamDec : Specifier VarDec { $$ = CreateNode(0, @$.first_line, "ParamDec", emp); BuildTree($$, 2, $1, $2); }
  ;


CompSt : LC DefList StmtList RC { $$ = CreateNode(0, @$.first_line, "CompSt", emp); BuildTree($$, 4, $1, $2, $3, $4); }
  ;

StmtList : Stmt StmtList { $$ = CreateNode(0, @$.first_line, "StmtList", emp); BuildTree($$, 2, $1, $2); }
  | /* epsilon */ { $$ = NULL; }
  | error RC { $$ = NULL; syntax_bool = 1; }
  ;

Stmt : Exp SEMI { $$ = CreateNode(0, @$.first_line, "Stmt", emp); BuildTree($$, 2, $1, $2); }
  | CompSt { $$ = CreateNode(0, @$.first_line, "Stmt", emp); BuildTree($$, 1, $1); }
  | RETURN Exp SEMI { $$ = CreateNode(0, @$.first_line, "Stmt", emp); BuildTree($$, 3, $1, $2, $3); }
  | WHILE LP Exp RP Stmt { $$ = CreateNode(0, @$.first_line, "Stmt", emp); BuildTree($$, 5, $1, $2, $3, $4, $5); }
  | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE { $$ = CreateNode(0, @$.first_line, "Stmt", emp); BuildTree($$, 5, $1, $2, $3, $4, $5); }
  | IF LP Exp RP Stmt ELSE Stmt { $$ = CreateNode(0, @$.first_line, "Stmt", emp); BuildTree($$, 7, $1, $2, $3, $4, $5, $6, $7); }   
  | RETURN error SEMI { $$ = NULL; syntax_bool = 1; }
  | IF LP error RP Stmt ELSE Stmt { $$ = NULL; syntax_bool = 1; }
  | IF LP Exp RP error ELSE Stmt { $$ = NULL; syntax_bool = 1; }
  | WHILE LP error RP Stmt { $$ = NULL; syntax_bool = 1; } 
  ;


DefList : Def DefList { $$ = CreateNode(0, @$.first_line, "DefList", emp); BuildTree($$, 2, $1, $2); }
  | /* epsilon */ { $$ = NULL; }
  ;

Def : Specifier DecList SEMI { $$ = CreateNode(0, @$.first_line, "Def", emp); BuildTree($$, 3, $1, $2, $3); }
  | Specifier error SEMI { $$ = NULL; syntax_bool = 1; } 
  ;

DecList : Dec { $$ = CreateNode(0, @$.first_line, "DecList", emp); BuildTree($$, 1, $1); }
  | Dec COMMA DecList { $$ = CreateNode(0, @$.first_line, "DecList", emp); BuildTree($$, 3, $1, $2, $3); }
  | Dec error { $$ = NULL; syntax_bool = 1; } 
  ;

Dec : VarDec { $$ = CreateNode(0, @$.first_line, "Dec", emp); BuildTree($$, 1, $1); }
  | VarDec ASSIGNOP Exp { $$ = CreateNode(0, @$.first_line, "Dec", emp); BuildTree($$, 3, $1, $2, $3); }
  | VarDec ASSIGNOP error { $$ = NULL; syntax_bool = 1; }
  ;


Exp : Exp ASSIGNOP Exp { $$ = CreateNode(0, @$.first_line, "Exp", emp); BuildTree($$, 3, $1, $2, $3); }
  | Exp AND Exp { $$ = CreateNode(0, @$.first_line, "Exp", emp); BuildTree($$, 3, $1, $2, $3); }
  | Exp OR Exp { $$ = CreateNode(0, @$.first_line, "Exp", emp); BuildTree($$, 3, $1, $2, $3); }
  | Exp RELOP Exp { $$ = CreateNode(0, @$.first_line, "Exp", emp); BuildTree($$, 3, $1, $2, $3); }
  | Exp PLUS Exp { $$ = CreateNode(0, @$.first_line, "Exp", emp); BuildTree($$, 3, $1, $2, $3); }
  | Exp MINUS Exp { $$ = CreateNode(0, @$.first_line, "Exp", emp); BuildTree($$, 3, $1, $2, $3); }
  | Exp STAR Exp { $$ = CreateNode(0, @$.first_line, "Exp", emp); BuildTree($$, 3, $1, $2, $3); }
  | Exp DIV Exp { $$ = CreateNode(0, @$.first_line, "Exp", emp); BuildTree($$, 3, $1, $2, $3); }
  | LP Exp RP { $$ = CreateNode(0, @$.first_line, "Exp", emp); BuildTree($$, 3, $1, $2, $3); }
  | MINUS Exp { $$ = CreateNode(0, @$.first_line, "Exp", emp); BuildTree($$, 2, $1, $2); }
  | NOT Exp { $$ = CreateNode(0, @$.first_line, "Exp", emp); BuildTree($$, 2, $1, $2); }
  | ID LP Args RP { $$ = CreateNode(0, @$.first_line, "Exp", emp); BuildTree($$, 4, $1, $2, $3, $4); }
  | ID LP RP { $$ = CreateNode(0, @$.first_line, "Exp", emp); BuildTree($$, 3, $1, $2, $3); }
  | Exp LB Exp RB { $$ = CreateNode(0, @$.first_line, "Exp", emp); BuildTree($$, 4, $1, $2, $3, $4); }
  | Exp DOT ID { $$ = CreateNode(0, @$.first_line, "Exp", emp); BuildTree($$, 3, $1, $2, $3); }
  | ID { $$ = CreateNode(0, @$.first_line, "Exp", emp); BuildTree($$, 1, $1); }
  | INT { $$ = CreateNode(0, @$.first_line, "Exp", emp); BuildTree($$, 1, $1); }
  | FLOAT { $$ = CreateNode(0, @$.first_line, "Exp", emp); BuildTree($$, 1, $1); }
  | Exp ASSIGNOP error SEMI  { $$ = NULL; syntax_bool = 1; }
  | Exp AND error SEMI   { $$ = NULL; syntax_bool = 1; }
  | Exp OR error SEMI   { $$ = NULL; syntax_bool = 1; }
  | Exp RELOP error SEMI   { $$ = NULL; syntax_bool = 1; }
  | Exp PLUS error SEMI  { $$ = NULL; syntax_bool = 1; }
  | Exp MINUS error SEMI   { $$ = NULL; syntax_bool = 1; }
  | Exp STAR error SEMI   { $$ = NULL; syntax_bool = 1; }
  | Exp DIV error SEMI   { $$ = NULL; syntax_bool = 1; }
  | ID LP error RP { $$ = NULL; syntax_bool = 1; }
  | Exp LB error RB { $$ = NULL; syntax_bool = 1; }
  ;

Args : Exp COMMA Args { $$ = CreateNode(0, @$.first_line, "Args", emp); BuildTree($$, 3, $1, $2, $3); }
  | Exp { $$ = CreateNode(0, @$.first_line, "Args", emp); BuildTree($$, 1, $1); }
  ;

%%

int yyerror(char *msg) {
   printf("Error type B at Line %d: %s.\n", yylineno, msg);
   syntax_bool = 1;
}
