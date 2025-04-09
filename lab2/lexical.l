%{
    #include "syntax.tab.h"
    #include "tree.h"
    #define YY_USER_ACTION \
            yylloc.first_line = yylloc.last_line = yylineno; \
            yylloc.first_column = yycolumn; \
            yylloc.last_column = yycolumn + yyleng - 1; \
            yycolumn += yyleng;
    int yycolumn = 1;
    extern int lexical_bool;
%}
%option yylineno
DIGIT [0-9]
LETTER [a-zA-Z]

NOP " "|"\r"|"\t"
LINE "\n"
OCOM "//"
LCOM "/*"

DOT "."
INT 0|([1-9]{DIGIT}*)
FLOAT {INT}{DOT}{DIGIT}+

COMMA ","
SEMI ";"
ASSIGNOP "="
NOT "!"
RELOP "=="|"!="|">"|"<"|">="|"<="
PLUS "+"
MINUS "-"
STAR "*"
DIV "/"
AND "&&"
OR "||"

LP "("
RP ")"
LB "["
RB "]"
LC "{"
RC "}"

TYPE "int"|"float"
IF "if"
ELSE "else"
WHILE "while"
RETURN "return"
STRUCT "struct"

ID (_|{LETTER})(_|{LETTER}|{DIGIT})*

%%

{NOP} {
}

{LINE} {
yycolumn = 1; 
}

{OCOM} {
char c  = input();
while(c != '\n') 
  c = input(); 
}

{LCOM} { 
char pre = input();
char cur = input();
while(cur != '\0' || cur != EOF) {
  if(pre == '*' && cur == '/') break;
  
  if(cur == '\0' || cur == EOF) {
     printf("Error type A at Line %d: Wrong on 2.3 \'*/\'\n", yylineno);
     lexical_bool = 1;
     break;
  }
  
  pre = cur;
  cur = input();
  } 

}

{FLOAT} {
yylval.nodetype = CreateNode(1, yylineno, "FLOAT", yytext); 
return FLOAT; 
}

{INT} {//printf("INT %s %d\n", yytext, yylineno);
yylval.nodetype = CreateNode(1, yylineno, "INT", yytext); 
return INT; 
}

{DOT} {
yylval.nodetype = CreateNode(1, yylineno, "DOT", yytext); 
return DOT; 
}

{COMMA} {
yylval.nodetype = CreateNode(1, yylineno, "COMMA", yytext); 
return COMMA; 
}

{SEMI} {
yylval.nodetype = CreateNode(1, yylineno, "SEMI", yytext); 
return SEMI; 
}

{ASSIGNOP} {
yylval.nodetype = CreateNode(1, yylineno, "ASSIGNOP", yytext); 
return ASSIGNOP; 
}

{NOT} {
yylval.nodetype = CreateNode(1, yylineno, "NOT", yytext); 
return NOT; 
}

{RELOP} {
yylval.nodetype = CreateNode(1, yylineno, "RELOP", yytext); 
return RELOP; 
}

{PLUS} {
yylval.nodetype = CreateNode(1, yylineno, "PLUS", yytext); 
return PLUS; 
}

{MINUS} {
yylval.nodetype = CreateNode(1, yylineno, "MINUS", yytext); 
return MINUS; 
}

{STAR} {
yylval.nodetype = CreateNode(1, yylineno, "STAR", yytext); 
return STAR; 
}

{DIV} {
yylval.nodetype = CreateNode(1, yylineno, "DIV", yytext); 
return DIV; 
}

{AND} {
yylval.nodetype = CreateNode(1, yylineno, "AND", yytext); 
return AND; 
}

{OR} {
yylval.nodetype = CreateNode(1, yylineno, "OR", yytext); 
return OR; 
}

{LP} {
yylval.nodetype = CreateNode(1, yylineno, "LP", yytext); 
return LP; 
}

{RP} {
yylval.nodetype = CreateNode(1, yylineno, "RP", yytext); 
return RP; 
}

{LB} {//printf("LB %s %d\n", yytext, yylineno);
yylval.nodetype = CreateNode(1, yylineno, "LB", yytext); 
return LB; 
}

{RB} {//printf("RB %s %d\n", yytext, yylineno);
yylval.nodetype = CreateNode(1, yylineno, "RB", yytext); 
return RB; 
}

{LC} {
yylval.nodetype = CreateNode(1, yylineno, "LC", yytext); 
return LC; 
}

{RC} {
yylval.nodetype = CreateNode(1, yylineno, "RC", yytext); 
return RC; }
          
{TYPE} {
yylval.nodetype= CreateNode(1, yylineno, "TYPE", yytext); 
return TYPE;
}

{IF} {
yylval.nodetype = CreateNode(1, yylineno, "IF", yytext); 
return IF;
}

{ELSE} {
yylval.nodetype = CreateNode(1, yylineno, "ELSE", yytext); 
return ELSE; 
}

{WHILE} {
yylval.nodetype = CreateNode(1, yylineno, "WHILE", yytext); 
return WHILE; 
}
          
{RETURN} {
yylval.nodetype = CreateNode(1, yylineno, "RETURN", yytext); 
return RETURN;
}

{STRUCT} {
yylval.nodetype = CreateNode(1, yylineno, "STRUCT", yytext); 
return STRUCT; 
}

{ID} {//printf("ID %s %d\n", yytext, yylineno);
yylval.nodetype = CreateNode(1, yylineno, "ID", yytext); 
return ID;
}

. { 
printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext); 
lexical_bool = 1;
}
    
%%
