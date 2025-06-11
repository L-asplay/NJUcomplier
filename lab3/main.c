#include "tree.h"
#include "syumatin.h"
#include "syntax.tab.h"
#include"translate.h"

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
    
    if(lexical_bool == 0 && syntax_bool == 0) 
    PrintTree(root, 0);
    do_analysis(root);

    InterCodes* ic = translate_ExtDefList(root->kids);
    FILE * f_output = fopen(argv[2], "w");
    if (!f) {          // 如果文件指针 f 是 NULL（打开文件失败）
        perror(argv[1]);  // 打印 argv[1]（文件名）和错误原因
        return 1;      // 返回错误码 1，表示程序异常终止
    }
    fprint_InterCodes(f_output, ic);
    return 0;
}


