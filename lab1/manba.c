#include "manba.h"

Node* CreateNode(int t, int l, char *lex, char *s) {
    
    Node *node = malloc(sizeof(Node));
    node->terminal = t;
    node->lnumber = l;

    int l1 = strlen(lex);
    node->lextype = malloc(l1 + 1);
    strcpy( node->lextype, lex);
    
    if (t == 1)  
     if (strcmp(lex, "TYPE") == 0 || strcmp(lex, "ID") == 0 ) {
        int l2 = strlen(s);
        node->strcode = malloc(l2 + 1);
        strcpy( node->strcode, s); 
    }

    if (strcmp(lex, "FLOAT") == 0) 
    sscanf(s, "%f", &(node->fval));
    else if (strcmp(lex, "INT") == 0) 
    sscanf(s, "%u", &(node->uval));
    
    node->bros = NULL;
    node->kids = NULL;

    return node;
}

void BuildTree(Node* r, int cnt, ...) {
    
    va_list args;
    va_start(args, cnt);

    Node *pre = NULL;
    
    while(cnt--) {
        Node *cur = va_arg(args, Node *);
        if (cur == NULL) continue;

        if (pre == NULL) r->kids = cur;
        else pre->bros = cur;
       
        pre = cur;
    }
    
    va_end(args);
}

void PrintTree(Node* r, int prefix) {

    if (r == NULL) return;

    for(int i = 0; i < prefix; i++) printf("  ");
    
    if (r->terminal == 0) 
        printf("%s (%d)\n", r->lextype, r->lnumber);
    else if (strcmp(r->lextype, "ID") == 0) 
        printf("ID: %s\n", r->strcode);
    else if (strcmp(r->lextype, "TYPE") == 0) 
        printf("TYPE: %s\n", r->strcode);
    else if (strcmp(r->lextype, "FLOAT") == 0) 
        printf("FLOAT: %f\n", r->fval);   
    else if (strcmp(r->lextype, "INT") == 0) 
        printf("INT: %u\n", r->uval);
    else 
        printf("%s\n", r->lextype);
        
    PrintTree(r->kids, prefix + 1);
    PrintTree(r->bros, prefix);
    
}
