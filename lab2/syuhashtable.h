#ifndef SYUHASHTABLE_H
#define SYUHASHTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct Type_* Type; 
typedef struct FieldList_* FieldList; 
typedef struct HashNodeVar_* HashNodeVar; 
typedef struct HashNodeStru_* HashNodeStru; 
typedef struct HashNodeFunc_* HashNodeFunc; 
 
struct Type_ 
{ 
  enum { BASIC, ARRAY, STRUCTURE } kind; 
  union 
  { 
    // 基本类型 
    int basic; // 0 to int 1 to float
    // 数组类型信息包括元素类型与数组大小构成 
    struct { Type elem; int size; } array; 
    // 结构体类型信息是一个链表 
    FieldList structure; 
  } ; 
}; 

struct FieldList_ 
{   // 域的名字 类型 下一个域 
    char* name;  
    Type type; 
    FieldList tail;
};
  
// nodes designed for HashTables
struct HashNodeVar_ {
    int level; 
    char* name;
    Type type;
    HashNodeVar table; 
    HashNodeVar stcak;
};

struct HashNodeStru_ {
    Type type;
    HashNodeStru tail; 
};

struct HashNodeFunc_ {
    char* name;
    Type retyp;
    FieldList argvs;
    HashNodeFunc tail;
};

// functions of HashTables
void AddStackEnv(int dept);
void DelStackEnv(int dept);

int CheckType(Type x, Type y);

HashNodeVar QueryVar(char* s);
HashNodeStru QueryStru(char* s);
HashNodeFunc QueryFunc(char* s);

void InsertVar(int l, char* s, Type ty);
void InsertStru(Type ty);
void InsertFunc(HashNodeFunc func);

#endif
