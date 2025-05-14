#include "intercode.h"

static int temp_cnt = 0;
static int temp_label = 0;

// 参数列表
static bool ParamTable[16384];
void registerParam(char* name) {
    ParamTable[hash_pjw(name)] = 1;
}
bool isregisteredParam(char* name) {
    return ParamTable[hash_pjw(name)];
}

// utils
int floor_log10(int k) {//计算一个十进制数的（位数-1），100返回2，10返回1
    if (k <= 0) return -1; // 处理非法输入（k 必须为正整数）
    int count = 0;
    while (k >= 10) {
        k = k / 10;
        count++;
    }
    return count;
}

// new
char* new_temp() {//修改了
    temp_cnt ++;
    size_t len = 2 + (int)floor_log10(temp_cnt) + 1;
    //len = 前缀长度(1) + 数字位数 + 终止符'\0'(1)
    //如t100需要1+3+1
    char* ans = malloc(len);
    memset(ans, 0, len);
    ans[0] = 't';
    sprintf(ans + 1, "%d", temp_cnt);
    return ans;
}
char* new_label() {
    temp_label ++;
    size_t len = 2 + (int)floor_log10(temp_label) + 1;
    //len = 前缀长度(1) + 数字位数 + 终止符'\0'(1)
    //如L100需要1+3+1
    char* ans = malloc(len);
    memset(ans, 0, len);
    ans[0] = 'L';
    sprintf(ans + 1, "%d", temp_label);
    return ans;
}

// InterCode

InterCodes* get_empty_InterCodes() {//创建空链表容器
    InterCodes* ans = malloc(sizeof(InterCodes));
    ans->head = NULL;
    ans->tail = NULL;
    return ans;
}

InterCodes* InterCode2InterCodes(InterCode* code) {//将单条指令包装为链表
    if(code == NULL) {
        return NULL;
    }
    InterCodes* codes = malloc(sizeof(InterCodes));
    //可去除断言，但为了调试方便保留
    assert(code->prev == NULL);
    assert(code->next == NULL);
    codes->head = code;
    codes->tail = code;
    return codes;
}

InterCode* new_InterCode(int kind, ...) {//可变参数构造指令
    va_list args;
    va_start(args, kind);

    InterCode* ic = (InterCode*)malloc(sizeof(InterCode));

    ic->kind = kind;
    ic->arg1 = ic->arg2 = ic->dest = NULL;
    ic->size = 0;
    ic->relop = NULL;

    switch(kind) {
        case IC_DEC:
            ic->dest = va_arg(args, char*);
            ic->size = va_arg(args, int);
            break;
        case IC_IFGOTO:
            ic->arg1 = va_arg(args, char*);
            ic->relop = va_arg(args, InterRelop*);
            ic->arg2 = va_arg(args, char*);
            ic->dest = va_arg(args, char*);
            break;
        case IC_ASSIGN:
        case IC_ADDROF:
        case IC_ASSIGNFROMADDR:
        case IC_ASSIGNTOADDR:
        case IC_ASSIGNCALL:
            ic->dest = va_arg(args, char*);
            ic->arg1 = va_arg(args, char*);
            break;
        case IC_LABEL:
        case IC_FUNCTION:
        case IC_GOTO:
        case IC_RETURN:
        case IC_ARG:
        case IC_PARAM:
        case IC_READ:
        case IC_WRITE:
            ic->dest = va_arg(args, char*);
            break;
        case IC_PLUS:
        case IC_MINUS:
        case IC_MUL:
        case IC_DIV:
            ic->dest = va_arg(args, char*);
            ic->arg1 = va_arg(args, char*);
            ic->arg2 = va_arg(args, char*);
            break;
        default:
            fprintf(stderr, "Invalid kind for new_InterCode: %d\n", kind);
            exit(0);
    }

    va_end(args);

    return ic;
}


void add_InterCode(InterCodes* target, InterCode* cur) {//将新指令cur连接到链表尾部
    if(target == NULL) {
        target = InterCode2InterCodes(cur);
        return ;
    }
    if(cur == NULL) {
        return ;
    }
    assert(cur->prev == NULL);
    assert(cur->next == NULL);

    if(target->head == NULL) {
        assert(target->tail == NULL);
        target->head = cur;
        target->tail = cur;
        return ;
    }
    assert(target->tail != NULL);
    cur->prev = target->tail;
    target->tail->next = cur;
    target->tail = cur;
}
void add_InterCodes(InterCodes* target, InterCodes* cur) {//将cur链表整体接入target尾部
    assert(target != NULL);
    assert(cur != NULL);
    
    if(cur->head == NULL) {
        assert(cur->tail == NULL);
        return ;
    }
    if(target->head == NULL) {
        assert(target->tail == NULL);
        target->head = cur->head;
        target->tail = cur->tail;
        return ;
    }
    assert(target->tail != NULL);
    assert(cur->head != NULL);
    target->tail->next = cur->head;
    cur->head->prev = target->tail;
    target->tail = cur->tail;
    return ;
}

// Output
void fprint_InterRelop(FILE* f, InterRelop* relop) {
    char* rel_str[6] = {"<", ">", "<=", ">=", "==", "!="};
    fprintf(f, "%s", rel_str[relop->kind]);
}
void fprint_InterCode(FILE* f, InterCode* code) {
    switch(code->kind) {
        case IC_DEC:
            fprintf(f, "DEC %s %d", code->dest, code->size);
            break;
        case IC_IFGOTO:
            fprintf(f, "IF %s ", code->arg1);
            fprint_InterRelop(f, code->relop);
            fprintf(f, " %s GOTO %s", code->arg2, code->dest);
            break;
        case IC_LABEL:
            fprintf(f, "LABEL %s :", code->dest);
            break;
        case IC_FUNCTION:
            fprintf(f, "FUNCTION %s :", code->dest);
            break;
        case IC_GOTO:
            fprintf(f, "GOTO %s", code->dest);
            break;
        case IC_RETURN:
            fprintf(f, "RETURN %s", code->dest);
            break;
        case IC_ARG:
            fprintf(f, "ARG %s", code->dest);
            break;
        case IC_PARAM:
            fprintf(f, "PARAM %s", code->dest);
            break;
        case IC_READ:
            fprintf(f, "READ %s", code->dest);
            break;
        case IC_WRITE:
            fprintf(f, "WRITE %s", code->dest);            
            break;
        case IC_ASSIGN:
            fprintf(f, "%s := %s", code->dest, code->arg1);
            break;
        case IC_ADDROF:
            fprintf(f, "%s := &%s", code->dest, code->arg1);
            break;
        case IC_ASSIGNFROMADDR:
            fprintf(f, "%s := *%s", code->dest, code->arg1);
            break;
        case IC_ASSIGNTOADDR:
            fprintf(f, "*%s := %s", code->dest, code->arg1);
            break;
        case IC_ASSIGNCALL:
            fprintf(f, "%s := CALL %s", code->dest, code->arg1);
            break;
        case IC_PLUS:
            fprintf(f, "%s := %s + %s", code->dest, code->arg1, code->arg2);
            break;
        case IC_MINUS:
            fprintf(f, "%s := %s - %s", code->dest, code->arg1, code->arg2);
            break;
        case IC_MUL:
            fprintf(f, "%s := %s * %s", code->dest, code->arg1, code->arg2);
            break;
        case IC_DIV:
            fprintf(f, "%s := %s / %s", code->dest, code->arg1, code->arg2);
            break;
        default:
            fprintf(stderr, "Invalid kind for new_InterCode: %d\n", code->kind);
            exit(0);
    }
}
void fprint_InterCodes(FILE* f, InterCodes* codes) {//按链表顺序向文件f中逐个输出指令
    if(codes == NULL) {
        return ;
    }
    InterCode* cur = codes->head;
    while(cur != NULL) {
        fprint_InterCode(f, cur);
        fprintf(f, "\n");
        cur = cur->next;
    }
}
