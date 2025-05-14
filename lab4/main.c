#include "tree.h"
#include "syumatin.h"
#include "syntax.tab.h"
#include"translate.h"
#include "object.h"

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
    init_func_read();
    init_func_write();
    
    if(lexical_bool == 0 && syntax_bool == 0) ;
    //PrintTree(root, 0);
    else return 0;
    
    do_analysis(root);

    FILE * f_output = fopen(argv[2], "w");
    if (!f_output) { perror(argv[2]); return 1; }

    InterCodes* ic = translate_ExtDefList(root->kids);
    
    MipsNode mc = translate_ic(ic);
    fprint_MipsCodes(f_output, mc);
    
    return 0;
}


