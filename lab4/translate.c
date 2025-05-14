#include"translate.h"

//#define bug printf("> %s (%d) <%s, %d>\n", node->lextype, node->lnumber, __FUNCTION__, __LINE__)

extern HashNodeVar VarTable[16384];
extern HashNodeStru StruTable[16384];
extern HashNodeFunc FuncTable[16384];

//自顶向下递归的翻译，实际上是自底向上的翻译，先把子节点翻译完成，再并入父节点的翻译代码

InterCodes* translate_ExtDefList(Node* node){
    if(node == NULL) return get_empty_InterCodes();
    assert(!strcmp(node->lextype, "ExtDefList"));

    InterCodes* ExtDef_codes = translate_ExtDef(TreeFind(node, "ExtDef"));
    InterCodes* ExtDefList_codes = translate_ExtDefList(TreeFind(node, "ExtDefList"));

    add_InterCodes(ExtDef_codes, ExtDefList_codes);
    return ExtDef_codes;
}

InterCodes* translate_ExtDef(Node* node) {
    //ExtDef → Specifier ExtDecList SEMI 表示全局变量的定义，例如“int global1, global2;”
    //ExtDef → Specifier FunDec CompSt 表示函数的定义，其中Specifier是返回类型，FunDec是函数头，CompSt表示函数体
    
    if(node == NULL) return get_empty_InterCodes();
    assert(!strcmp(node->lextype, "ExtDef"));
    
    //杜绝了 结构体为参数 或者 结构体为返回值 的函数
    if(TreeFind(node->kids,"StructSpecifier")!=NULL){
        printf("cannot translate:code cannot include struct\n");
        exit(0);
    }
    Node* FunDec_node = TreeFind(node, "FunDec");
    if(FunDec_node != NULL) {
        InterCodes* LABEL_codes = InterCode2InterCodes(new_InterCode(IC_FUNCTION, FunDec_node->kids->strcode));//函数名字
        InterCodes* FunDec_codes = translate_FunDec(FunDec_node);
        InterCodes* CompSt_codes = translate_CompSt(TreeFind(node, "CompSt"));
        add_InterCodes(FunDec_codes, CompSt_codes);
        add_InterCodes(LABEL_codes, FunDec_codes);
        return LABEL_codes;
    } else {
        return get_empty_InterCodes();
    }
}

InterCodes* translate_FunDec(Node* node) {
    if(node == NULL) return get_empty_InterCodes();
    assert(!strcmp(node->lextype, "FunDec"));

    Node* VarList_node = TreeFind(node, "VarList");
    return translate_VarList(VarList_node);
}

InterCodes* translate_VarList(Node* node) {
    if(node == NULL) return get_empty_InterCodes();
    assert(!strcmp(node->lextype, "VarList"));

    InterCodes* ParamDec_codes = translate_ParamDec(TreeFind(node, "ParamDec"));
    InterCodes* VarList_codes = translate_VarList(TreeFind(node, "VarList"));
    add_InterCodes(ParamDec_codes, VarList_codes);
    return ParamDec_codes;
}

InterCodes* translate_ParamDec(Node* node) {
    if(node == NULL) return get_empty_InterCodes();
    assert(!strcmp(node->lextype, "ParamDec"));
    if(TreeFind(node->kids,"StructSpecifier")!=NULL){
        printf("cannot translate:code cannot include struct");
        exit(0);
    }

    return translate_VarDec_function(TreeFind(node, "VarDec"));
}

InterCodes* translate_VarDec_function(Node* node) {
    //一维数组传参有点关系但不大
    if(node == NULL) return get_empty_InterCodes();
    assert(!strcmp(node->lextype, "VarDec"));

    if(!strcmp(node->kids->lextype, "ID")) {//普通int类型的传参
        registerParam(node->kids->strcode);
        return InterCode2InterCodes(new_InterCode(IC_PARAM, node->kids->strcode));
    } 
    else if(!strcmp(node->kids->kids->lextype, "ID")){//其实和上面一样
        //int a[]传参
        char* id=node->kids->kids->strcode;
        assert(QueryVar(id)!=NULL);
        Type ty = QueryVar(id)->type;
        assert(ty->kind==ARRAY);
        registerParam(id);
        //地址的传参怎么写？只要写RARAM v1即可
        return InterCode2InterCodes(new_InterCode(IC_PARAM, id));
    }
    else {
        return translate_VarDec_function(TreeFind(node, "VarDec"));
    }
}

InterCodes* translate_CompSt(Node* node) {
    //语句块
    if(node == NULL) return get_empty_InterCodes();
    assert(!strcmp(node->lextype, "CompSt"));

    InterCodes* DefList_codes = translate_DefList(TreeFind(node, "DefList"));
    InterCodes* StmtList_codes = translate_StmtList(TreeFind(node, "StmtList"));
    add_InterCodes(DefList_codes, StmtList_codes);
    return DefList_codes;
}

InterCodes* translate_DefList(Node* node) {
    if(node == NULL) return get_empty_InterCodes();
    assert(!strcmp(node->lextype, "DefList"));
    
    InterCodes* Def_codes = translate_Def(TreeFind(node, "Def"));
    InterCodes* DefList_codes = translate_DefList(TreeFind(node, "DefList"));
    add_InterCodes(Def_codes, DefList_codes);
    return Def_codes;
}


InterCodes* translate_Def(Node* node) {
    if(node == NULL) return get_empty_InterCodes();
    assert(!strcmp(node->lextype, "Def"));
    if(TreeFind(node->kids,"StructSpecifier")!=NULL){
        printf("cannot translate:code cannot include struct");
        exit(0);
    }
    return translate_DecList(TreeFind(node, "DecList"));
}


InterCodes* translate_DecList(Node* node) {
    if(node == NULL) return get_empty_InterCodes();
    assert(!strcmp(node->lextype, "DecList"));

    InterCodes* Dec_codes = translate_Dec(TreeFind(node, "Dec"));
    InterCodes* DecList_codes = translate_DecList(TreeFind(node, "DecList"));
    add_InterCodes(Dec_codes, DecList_codes);
    return Dec_codes;
}


InterCodes* translate_Dec(Node* node){
    //对于那些类型不是数组或结构体的变量，直接使用即可，不需要使用DEC语句对其进行声明。
    //高维数组声明在这
    if(node == NULL) return get_empty_InterCodes();
    assert(!strcmp(node->lextype, "Dec"));
    Node*cur =node;
    
    for(;strcmp(cur->lextype, "ID")!=0;cur=cur->kids){}
    assert(cur!=NULL);
    char *id=cur->strcode;//找到VarDEC对应的id

    HashNodeVar t = QueryVar(id);
    assert(t!=NULL);
    Type ty=t->type;
    InterCodes* result = get_empty_InterCodes();

    if(t->type->kind == STRUCTURE||t->type->kind == ARRAY) {
        add_InterCode(result, new_InterCode(IC_DEC, id, getTypeSize(ty)));
    } 

    if (TreeFind(node, "ASSIGNOP") != NULL){//赋值操作
        if(ty->kind==BASIC)
        {
            char* t1 = new_temp();
            int *ppp=malloc(sizeof(int));
            InterCodes* Exp_codes = translate_Exp(TreeFind(node, "Exp"), t1,ppp);//将EXP的值赋给t1
            add_InterCode(Exp_codes, new_InterCode(IC_ASSIGN, id, t1));
            return Exp_codes;
        }
        else//数组的整体赋值，支持高维数组，并不需要
        {
            printf("不支持数组的整体赋值。\n");
            exit(0);
            // char* t1 = new_temp();
            // InterCodes* Exp_codes = translate_Exp(TreeFind(node, "Exp"), t1);
            // add_InterCodes(result, Exp_codes);
           
            // char* laddr = new_temp();// 目标变量地址 left adddress
            // char* raddr = t1;// 源数据地址 right address
            // add_InterCode(result, new_InterCode(IC_ADDROF, laddr, id));// laddr = &x

            // int tysize = getTypeSize(ty) >> 2;// 计算总大小（按4字节单位）
            // char* t_temp = new_temp();// 临时存储每个字的值
            
            // // 逐字拷贝内存
            // for(int i = 1; i <= tysize; i++) {
            //     add_InterCode(result, new_InterCode(IC_ASSIGNFROMADDR, t_temp, raddr));// t_temp = *raddr
            //     add_InterCode(result, new_InterCode(IC_ASSIGNTOADDR, laddr, t_temp));// *laddr = t_temp
            //     if(i != tysize) {
            //         add_InterCode(result, new_InterCode(IC_PLUS, laddr, laddr, "#4"));// laddr += 4
            //         add_InterCode(result, new_InterCode(IC_PLUS, raddr, raddr, "#4"));// raddr += 4
            //     }
            // }
            // return result;
        }
    }
    return result;
}


InterCodes* translate_StmtList(Node* node) {
    if(node == NULL) return get_empty_InterCodes();
    assert(!strcmp(node->lextype, "StmtList"));
    
    InterCodes* Stmt_codes = translate_Stmt(TreeFind(node, "Stmt"));
    InterCodes* StmtList_codes = translate_StmtList(TreeFind(node, "StmtList"));
    add_InterCodes(Stmt_codes, StmtList_codes);
    return Stmt_codes;
}


InterCodes* translate_Stmt(Node* node){
    if(node == NULL) return get_empty_InterCodes();
    assert(!strcmp(node->lextype, "Stmt"));
    /*
    Stmt → Exp SEMI 
        | CompSt 
        | RETURN Exp SEMI 
        | IF LP Exp RP Stmt 
        | IF LP Exp RP Stmt ELSE Stmt 
        | WHILE LP Exp RP Stmt
    */

    // Exp SEMI
    if(!strcmp(node->kids->lextype, "Exp")) {
        int *p=malloc(sizeof(int));
        return translate_Exp(TreeFind(node, "Exp"), "tNULL",p);
    }
    // CompSt
    if(TreeFind(node, "CompSt")) {
        return translate_CompSt(node->kids);
    }
    // RETURN Exp SEMI
    if(TreeFind(node, "RETURN")) {
        char* t1 = new_temp();
        int *p=malloc(sizeof(int));
        InterCodes* codes = translate_Exp(TreeFind(node, "Exp"), t1,p);
        if(*p==1){
            add_InterCode(codes, new_InterCode(IC_ASSIGNFROMADDR, t1,t1));
        }
        add_InterCode(codes, new_InterCode(IC_RETURN, t1));
        return codes;
    }
    // IF LP Exp RP Stmt
    if(TreeFind(node, "IF") && !TreeFind(node, "ELSE")) {
        char* label1 = new_label();
        char* label2 = new_label();
        InterCodes* code1 = translate_Cond(TreeFind(node, "Exp"), label1, label2);
        InterCodes* code2 = translate_Stmt(TreeFind(node, "Stmt"));
        add_InterCode(code1, new_InterCode(IC_LABEL, label1));
        add_InterCodes(code1, code2);
        add_InterCode(code1, new_InterCode(IC_LABEL, label2));
        return code1;
    }
    // IF LP Exp RP Stmt ELSE Stmt
    if(TreeFind(node, "ELSE")) {
        char* label1 = new_label();
        char* label2 = new_label();
        char* label3 = new_label();
        InterCodes* code1 = translate_Cond(TreeFind(node, "Exp"), label1, label2);
        InterCodes* code2 = translate_Stmt(node->kids->bros->bros->bros->bros);//注意这里用TreeFind只能够找到第一个Stmt
        InterCodes* code3 = translate_Stmt(node->kids->bros->bros->bros->bros->bros->bros);
        add_InterCode(code1, new_InterCode(IC_LABEL, label1));
        add_InterCodes(code1, code2);
        add_InterCode(code1, new_InterCode(IC_GOTO, label3));
        add_InterCode(code1, new_InterCode(IC_LABEL, label2));
        add_InterCodes(code1, code3);
        add_InterCode(code1, new_InterCode(IC_LABEL, label3));
        return code1;
    }
    // WHILE LP Exp RP Stmt
    if(TreeFind(node, "WHILE")) {
        char* label1 = new_label();
        char* label2 = new_label();
        char* label3 = new_label();
        InterCodes* code1 = translate_Cond(TreeFind(node, "Exp"), label2, label3);
        InterCodes* code2 = translate_Stmt(TreeFind(node, "Stmt"));

        InterCodes* result = InterCode2InterCodes(new_InterCode(IC_LABEL, label1));
        add_InterCodes(result, code1);
        add_InterCode(result, new_InterCode(IC_LABEL, label2));
        add_InterCodes(result, code2);
        add_InterCode(result, new_InterCode(IC_GOTO, label1));
        add_InterCode(result, new_InterCode(IC_LABEL, label3));
        return result;
    }
    assert(0);
}

char* int_to_hashtag(int num) {//给定整数1,输出"#1"
    // 计算格式化后的字符串长度（包括'#'和'\0'）
    int length = snprintf(NULL, 0, "#%d", num) + 1;  // +1 for null terminator

    // 动态分配内存
    char* result = (char*)malloc(length * sizeof(char));
    if (result == NULL) {
        return NULL;  // 内存分配失败
    }

    // 将整数格式化为 "#数值" 字符串
    snprintf(result, length, "#%d", num);
    return result;
}

InterCodes* translate_Exp(Node* node, char* place,int *is){//place是存放返回值的字符串，is指针存放的是0代表数值，1代表地址
    //左值的Exp1的为以下三种情况之一：单个变量访问、数组元素访问（或结构体特定域的访问）
    if(node == NULL) return get_empty_InterCodes();
    assert(!strcmp(node->lextype, "Exp"));
    // INT
    
    if(!strcmp(node->kids->lextype, "INT")) {
        *is=0;
    	//printf("reach2 here\n");
        //读取Uval，返回#uval字符串
        int val=node->kids->uval;
        char *int_str=int_to_hashtag(val);
        //printf("reach3 here\n");
        return InterCode2InterCodes(new_InterCode(IC_ASSIGN, place, int_str));
    }
    // ID
    if(!strcmp(node->kids->lextype, "ID") && node->kids->bros == NULL) {
        *is=0;
    	//printf("reach1 here\n");
        char* id = node->kids->strcode;
        assert(QueryVar(id)!=NULL);
        if(QueryVar(id)->type->kind != BASIC) {//非基本类型是函数的参数
            //一维数组传参，直接传递地址
            return InterCode2InterCodes(new_InterCode(IC_ADDROF, place, id));
        } else {
            return InterCode2InterCodes(new_InterCode(IC_ASSIGN, place, id));
        }
    }   
    /*
    Exp1 ASSIGNOP Exp2
        (Exp1 → ID) 
    */
    
   if(TreeFind(node, "ASSIGNOP") != NULL){
   	assert(is!=NULL);
        *is=0;
        //printf("reach1 here\n");
        char* tval = new_temp();
        int *right0=malloc(sizeof(int));//表明返回place的是数值还是地址
        InterCodes* RExp_codes = translate_Exp(node->kids->bros->bros, tval,right0);//Right Exp
        char* rval=new_temp();//存储右值返回的数值
        assert(*right0==0||*right0==1);
        if(*right0==0){//右值是数值
            add_InterCode(RExp_codes, new_InterCode(IC_ASSIGN, rval, tval));
        }
        else if(*right0==1){//右值是地址
            add_InterCode(RExp_codes, new_InterCode(IC_ASSIGNFROMADDR, rval, tval));
        }
        if(!strcmp(node->kids->kids->lextype, "ID")) {
            //一定是单个变量的访问 如int a=1;
            char* id = node->kids->kids->strcode;
            assert(QueryVar(id)!=NULL);
            Type ty = QueryVar(id)->type;
            if(ty->kind == BASIC) {
                add_InterCode(RExp_codes, new_InterCode(IC_ASSIGN, id, rval));
                add_InterCode(RExp_codes, new_InterCode(IC_ASSIGN, place, id));
                return RExp_codes;
            }        
            else{
                printf("this type is not allowed in Exp Assign Exp!\n");
                exit(0);
            }
        }

        else if(TreeFind(node->kids,"LB")!=NULL){
            //数组访问赋值 如a[2]=1;a[1][3]=6;
            int *left0=malloc(sizeof(int));
            char* lval = new_temp();//lval存储翻译出来的地址p
            InterCodes* LExp_codes=translate_Exp(node->kids,lval,left0);//lval存储翻译出来的地址p
            assert(*left0==1);
            add_InterCodes(RExp_codes,LExp_codes);

            add_InterCode(RExp_codes,new_InterCode(IC_ASSIGNTOADDR, lval, rval));
            add_InterCode(RExp_codes, new_InterCode(IC_ASSIGNFROMADDR, place, lval));
            return RExp_codes;
        }
    }
    //Exp LB Exp RB 数组访问表达式，包含高维数组的访问
    //（Exp DOT ID 结构体访问表达不需要写）
    if(TreeFind(node, "LB")!=NULL)
    {
        *is=1;
        InterCodes* result=get_empty_InterCodes();
        Node* cur = node;
        while(strcmp(cur->lextype, "ID")) cur = cur->kids;
        char*id = cur->strcode;
    
        if(isregisteredParam(id)) {
            add_InterCode(result,(new_InterCode(IC_ASSIGN, place, id)));
        } else {
            //首地址赋值给place
            add_InterCode(result,(new_InterCode(IC_ADDROF, place, id)));
        }
        //计算偏移地址
        cur = node;  // cur当前节点为高维数组的 EXP 父节点
        assert(QueryVar(id)!=NULL);
        
        Type tmp_type = QueryVar(id)->type;  // 查询符号表获取数组类型信息
        int cur_dimension=4;//从上往下走，最开始的基础类型是4，每次乘上数组那一维的size
        int array_depth=-1;
        while(tmp_type!=NULL){
            tmp_type=tmp_type->array.elem;
            array_depth++; 
        }
        char* offset = new_temp();
        InterCodes *index_codes1=InterCode2InterCodes(new_InterCode(IC_ASSIGN, offset, "#0"));
        add_InterCodes(result,index_codes1);
        // 循环处理每个维度
        while (TreeFind(cur, "LB") != NULL) {
            // 提取当前维度的索引表达式（如 i）
            Node* index_exp = cur->kids->bros->bros;
            
            // 生成计算当前索引值的中间代码
            char* t_index = new_temp();
            int *pp=malloc(sizeof(int));
            InterCodes* index_codes = translate_Exp(index_exp, t_index,pp);
            add_InterCodes(result,index_codes);
            assert(*pp==0);
            // 计算当前维度的偏移量
            char * this_dim=new_temp();
            add_InterCode(result, new_InterCode(IC_MUL, this_dim, t_index,int_to_hashtag(cur_dimension)));
            add_InterCode(result, new_InterCode(IC_PLUS,offset,offset, this_dim));
            // 更新cur_dimension到下一维度(更高维度)
            array_depth--;
            Type t= QueryVar(id)->type; 
            for(int i=0;i<array_depth;i++){
                t=t->array.elem;
            }
            cur_dimension=t->array.size*cur_dimension;
            cur=cur->kids;
        }
        add_InterCode(result, new_InterCode(IC_PLUS,place,place,offset));
        return result;
    }

    //MINUS Exp
    if(strcmp(node->kids->lextype, "MINUS")==0) {
        *is=0;
        char* t1 = new_temp();
        int *p=malloc(sizeof(int));
        InterCodes* codes = translate_Exp(node->kids->bros, t1,p);
        if(*p==0){
            add_InterCode(codes, new_InterCode(IC_MINUS, place, "#0", t1));
        }
        else if(*p==1){
            add_InterCode(codes, new_InterCode(IC_ASSIGNFROMADDR, t1, t1));
            add_InterCode(codes, new_InterCode(IC_MINUS, place, "#0", t1));
        }
        
        return codes;
    }

    //Exp PLUS Exp 
    if(TreeFind(node, "PLUS") != NULL) {
        *is=0;
        char* t1 = new_temp();
        char* t2 = new_temp();
        int *p1=malloc(sizeof(int));
        int *p2=malloc(sizeof(int));
        InterCodes* codes1 = translate_Exp(node->kids, t1,p1);
        InterCodes* codes2 = translate_Exp(node->kids->bros->bros, t2,p2);
        add_InterCodes(codes1, codes2);
        if(*p1==1){
            add_InterCode(codes1, new_InterCode(IC_ASSIGNFROMADDR, t1, t1));
        }
        if(*p2==1){
            add_InterCode(codes1, new_InterCode(IC_ASSIGNFROMADDR, t2, t2));
        }
        add_InterCode(codes1, new_InterCode(IC_PLUS, place, t1, t2));
        return codes1;
    }
    //Exp MINUS Exp
    if(TreeFind(node, "MINUS") != NULL && strcmp(node->kids->lextype, "MINUS")!=0) {
        //避免与MINUS EXP混淆
        *is=0;
        char* t1 = new_temp();
        char* t2 = new_temp();
        int *p1=malloc(sizeof(int));
        int *p2=malloc(sizeof(int));
        InterCodes* codes1 = translate_Exp(node->kids, t1,p1);
        InterCodes* codes2 = translate_Exp(node->kids->bros->bros, t2,p2);
        add_InterCodes(codes1, codes2);
        if(*p1==1){
            add_InterCode(codes1, new_InterCode(IC_ASSIGNFROMADDR, t1, t1));
        }
        if(*p2==1){
            add_InterCode(codes1, new_InterCode(IC_ASSIGNFROMADDR, t2, t2));
        }
        add_InterCode(codes1, new_InterCode(IC_MINUS, place, t1, t2));
        return codes1;
    }
    // Exp STAR Exp 
    if(TreeFind(node, "STAR") != NULL) {
        *is=0;
        char* t1 = new_temp();
        char* t2 = new_temp();
        int *p1=malloc(sizeof(int));
        int *p2=malloc(sizeof(int));
        InterCodes* codes1 = translate_Exp(node->kids, t1,p1);
        InterCodes* codes2 = translate_Exp(node->kids->bros->bros, t2,p2);
        add_InterCodes(codes1, codes2);
        if(*p1==1){
            add_InterCode(codes1, new_InterCode(IC_ASSIGNFROMADDR, t1, t1));
        }
        if(*p2==1){
            add_InterCode(codes1, new_InterCode(IC_ASSIGNFROMADDR, t2, t2));
        }
        add_InterCode(codes1, new_InterCode(IC_MUL, place, t1, t2));
        return codes1;
    }
    //Exp DIV Exp 
    if(TreeFind(node, "DIV") != NULL) {
    	*is=0;
        char* t1 = new_temp();
        char* t2 = new_temp();
        int *p1=malloc(sizeof(int));
        int *p2=malloc(sizeof(int));
        InterCodes* codes1 = translate_Exp(node->kids, t1,p1);
        InterCodes* codes2 = translate_Exp(node->kids->bros->bros, t2,p2);
        add_InterCodes(codes1, codes2);
        if(*p1==1){
            add_InterCode(codes1, new_InterCode(IC_ASSIGNFROMADDR, t1, t1));
        }
        if(*p2==1){
            add_InterCode(codes1, new_InterCode(IC_ASSIGNFROMADDR, t2, t2));
        }
        add_InterCode(codes1, new_InterCode(IC_DIV, place, t1, t2));
        return codes1;
    }
    // RELOP NOT AND OR
    if(TreeFind(node, "RELOP")||TreeFind(node, "NOT") ||TreeFind(node, "AND") || TreeFind(node, "OR") ) {
    	*is=0;
        char* L1 = new_label();
        char* L2 = new_label();
        InterCodes* result = InterCode2InterCodes(new_InterCode(IC_ASSIGN, place, "#0"));
        add_InterCodes(result , translate_Cond(node, L1, L2));
        add_InterCode(result , new_InterCode(IC_LABEL, L1));
        add_InterCode(result , new_InterCode(IC_ASSIGN, place, "#1"));
        add_InterCode(result , new_InterCode(IC_LABEL, L2));
        return result;
    }
    //LP Exp RP 
    if(strcmp(node->kids->lextype,"LP")==0){
        return translate_Exp(node->kids->bros,place,is);
    }
    //ID LP RP 无参数的函数调用
    //read函数没有任何参数，返回值为int型（即读入的整数值）
    if(strcmp(node->kids->lextype,"ID")==0
    &&strcmp(node->kids->bros->lextype,"LP")==0
    &&!TreeFind(node,"Args"))
    {
    	*is=0;
        char* id = node->kids->strcode;
        //查表,一定要有函数（修改了）
        if(QueryFunc(id)==NULL)assert(0);
        if(!strcmp(id, "read")) {
            return InterCode2InterCodes(new_InterCode(IC_READ, place));
        }
        return InterCode2InterCodes(new_InterCode(IC_ASSIGNCALL, place, id));
    }

    //ID LP Args RP 
    //write函数包含一个int类型的参数（即要输出的整数值），返回值也为int型（固定返回0）
    if(strcmp(node->kids->lextype,"ID")==0
    &&strcmp(node->kids->bros->lextype,"LP")==0
    &&strcmp(node->kids->bros->bros->lextype,"Args")==0)
    {
    	*is=0;
        char* id = node->kids->strcode;
        //查表,一定要有函数（修改了）
        if(QueryFunc(id)==NULL)assert(0);
        ArgList* arg_list = malloc(sizeof(ArgList));
        arg_list->head = NULL;
        InterCodes* codes1 = translate_Args(TreeFind(node, "Args"), arg_list);
        if(strcmp(id, "write")==0) {
            add_InterCode(codes1, new_InterCode(IC_WRITE, arg_list->head->ID));
            add_InterCode(codes1, new_InterCode(IC_ASSIGN, place, "#0"));
            return codes1;
        }
        InterCodes* codes2 = get_empty_InterCodes();
        ArgID* cur_arg = arg_list->head;

        while(cur_arg != NULL) {
            InterCodes* cur_codes = InterCode2InterCodes(new_InterCode(IC_ARG, cur_arg->ID));
            add_InterCodes(codes2, cur_codes);
            cur_arg = cur_arg->next;
        }
        add_InterCodes(codes1, codes2);
        add_InterCode(codes1, new_InterCode(IC_ASSIGNCALL, place, id));
        return codes1;

    }
    
}

InterCodes* translate_Cond(Node* node, char* label_true, char* label_false){
    //Cond(条件表达式)
    if(node == NULL) return get_empty_InterCodes();
    assert(!strcmp(node->lextype, "Exp"));
    //Exp1 RELOP Exp2 
    if(TreeFind(node, "RELOP") != NULL) {
        char* t1 = new_temp();
        char* t2 = new_temp();
        int *p1=malloc(sizeof(int));
        int *p2=malloc(sizeof(int));
        InterCodes* codes1 = translate_Exp(node->kids, t1,p1);
        InterCodes* codes2 = translate_Exp(node->kids->bros->bros, t2,p2);
        char* relop_str = node->kids->bros->strcode;
	    assert(relop_str!=NULL);

        InterRelop* op = malloc(sizeof(InterRelop));
        if(!strcmp(relop_str, "<")) op->kind = REL_LT;
        if(!strcmp(relop_str, ">")) op->kind = REL_GT;
        if(!strcmp(relop_str, "<=")) op->kind = REL_LEQ;
        if(!strcmp(relop_str, ">=")) op->kind = REL_GEQ;
        if(!strcmp(relop_str, "==")) op->kind = REL_EQ;
        if(!strcmp(relop_str, "!=")) op->kind = REL_NEQ;
        add_InterCodes(codes1, codes2);
        if(*p1==1){
            add_InterCode(codes1, new_InterCode(IC_ASSIGNFROMADDR, t1, t1));
        }
        if(*p2==1){
            add_InterCode(codes1, new_InterCode(IC_ASSIGNFROMADDR, t2, t2));
        }
        add_InterCode(codes1, new_InterCode(IC_IFGOTO, t1, op, t2, label_true));
        add_InterCode(codes1, new_InterCode(IC_GOTO, label_false));
        return codes1;
    } 
    //NOT Exp1
    else if (strcmp(node->kids->lextype,"NOT")==0) {
        return translate_Cond(node->kids->bros, label_false, label_true);
    } 
    //Exp1 AND Exp2 
    else if (TreeFind(node, "AND")) {
        char* L1 = new_label();
        InterCodes* codes1 = translate_Cond(node->kids, L1, label_false);
        InterCodes* codes2 = translate_Cond(node->kids->bros->bros, label_true, label_false);
        add_InterCode(codes1, new_InterCode(IC_LABEL, L1));
        add_InterCodes(codes1, codes2);
        return codes1;
    } 
    //Exp1 OR Exp2
    else if (TreeFind(node, "OR")) {
        char* L1 = new_label();
        InterCodes* codes1 = translate_Cond(node->kids, label_true, L1);
        InterCodes* codes2 = translate_Cond(node->kids->bros->bros, label_true, label_false);
        add_InterCode(codes1, new_InterCode(IC_LABEL, L1));
        add_InterCodes(codes1, codes2);
        return codes1;
    } 
    //(other cases) if(x)
    else {
        char* t1 = new_temp();
        int *p=malloc(sizeof(int));
        InterCodes* codes = translate_Exp(node, t1,p);
        InterRelop* op = malloc(sizeof(InterRelop));
        op->kind = REL_NEQ;
        if(*p==1)add_InterCode(codes, new_InterCode(IC_ASSIGNFROMADDR, t1, t1));
        add_InterCode(codes, new_InterCode(IC_IFGOTO, t1, op, "#0", label_true));
        add_InterCode(codes, new_InterCode(IC_GOTO, label_false));
        return codes;
    }
}

/*
Args → Exp COMMA Args | Exp 
*/
InterCodes* translate_Args(Node* node, ArgList* arg_list) {
    //将参数值存放在arg_list中
    char* t1 = new_temp();
    int *p=malloc(sizeof(int));
    InterCodes* codes1 = translate_Exp(node->kids, t1,p);
    
    if(*p==1){
        //如果t1是地址，需要把他的值取出来再传参
        add_InterCode(codes1,new_InterCode(IC_ASSIGNFROMADDR,t1,t1));
    }
    ArgID* cur = malloc(sizeof(ArgID));
    cur->next = NULL;
    cur->ID = t1;
    if(arg_list->head == NULL) {
        arg_list->head = cur;
    } 
    else {
    //新的参数插在头部，以确保实参是反的
    cur->next = arg_list->head;
    arg_list->head = cur;
    }
    if(TreeFind(node, "COMMA")==NULL)return codes1;
    
    InterCodes* codes2 = translate_Args(TreeFind(node, "Args"), arg_list);
    add_InterCodes(codes1, codes2);
    return codes1;

}

// 辅助函数：获取基本类型的字节大小（int为4字节）
int get_element_size(Type type) {
    while (type->kind == ARRAY) {
        type = type->array.elem;
    }
    if (type->kind == BASIC) {
        return 4;  // int占4字节
    }
    return 0;  // 结构体暂不支持
}

// 递归计算当前维度的步长（当前维度大小乘以内层总步长）
int calculate_stride(Type current_type) {
    if (current_type->kind == ARRAY) {
        return current_type->array.size * calculate_stride(current_type->array.elem);
    } else {
        return get_element_size(current_type);  // 基本类型步长即元素int大小
    }
}

// 递归计算高维数组偏移量（需配合索引数组）
int calculate_offset(Type array_type, int* indices, int dim_index) {
    //从数组首地址那一维开始计算，首次调用dim_index=0
    if (array_type->kind != ARRAY) {
        return 0;  // 递归终止条件：非数组类型
    }
    
    // 计算当前维度的步长（如 a[i][j] 中 i 的步长 = j的维度数 * 元素大小）
    int stride = calculate_stride(array_type->array.elem->array.elem);
    
    // 当前维度贡献：indices[dim_index] * stride
    int current_offset = indices[dim_index] * stride;
    
    // 递归计算后续维度的偏移量
    return current_offset + calculate_offset(array_type->array.elem, indices, dim_index + 1);
}


