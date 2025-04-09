#include "tree.h"
#include "syumatin.h"
#include "syntax.tab.h"
int lexical_bool = 0;
int syntax_bool = 0;
Node* root = NULL;
extern void yyrestart  (FILE * input_file );
extern int yyparse (void);

int main(int argc, char **argv) {
    if(argc <= 1)  return 1;
   
    FILE * f = fopen(argv[1], "r");
    if(!f) { perror(argv[1]); return 1;}
    
    yyrestart(f); yyparse();
    
    if(lexical_bool == 0 && syntax_bool == 0) 
    //PrintTree(root, 0);
    do_analysis(root);
    return 0;
}

