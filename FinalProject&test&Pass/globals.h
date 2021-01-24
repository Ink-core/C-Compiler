#ifndef GLOBALS_H
#define GLOBALS_H
#include <iostream>
using namespace std;

#ifndef YYPARSER
#include "parser.hpp"
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define MAXRESERVED 8

typedef int TokenType;


extern FILE* source; //测试文件
extern FILE* listing; //输出


extern int lineno; 

/**************************************************/
/*********** Syntax tree for parsing **************/
/**************************************************/
//枚举类型：枚举各节点类型的对应整数 Nodekind
typedef enum {
    ErrorK,
	
	ProgramK, 
    VariableDeclarationK,
    ArrayDeclarationK,
    FunctionDeclarationK,

    VariableParameterK,
    ArrayParameterK,

    CompoundStatementK,
    ExpressionStatementK,
    SelectionStatementK,
    IterationStatementK,
    ReturnStatementK,

    AssignExpressionK,
    ComparisonExpressionK,
    AdditiveExpressionK,
    MultiplicativeExpressionK,

    VariableK,
    ArrayK,
    CallK,

    ConstantK,
    TokenTypeK,
} NodeKind;

//定义树的节点
typedef struct treeNode {
  struct treeNode *sibling;//兄弟节点
  int lineno;//所在行数
  NodeKind nodeKind;//节点类型
    union {//节点属性
  	  //ProgramK 程序节点
  	  struct{
  		    struct treeNode *cur;	
	    } Pro;
      // VariableDeclarationK 变量声明节点
      struct {
          struct treeNode *type_spec;
          struct treeNode *_var;
      } varDecl;

      // ArrayDeclarationK 数组声明
      struct {
          struct treeNode *type_spec;
          struct treeNode *_var;
          struct treeNode *_num;
      } arrDecl;

      // FunctionDeclarationK 函数声明
      struct {
          struct treeNode *type_spec; //返回值类型
          struct treeNode *_var;    //函数名字
          struct treeNode *params;  //参数
          struct treeNode *cmpd_stmt;  //复合语句
      } funcDecl;

      // VariableParameterK 参数变量(注意根据语法int a,b是错误的)
      struct {
          struct treeNode *type_spec;
          struct treeNode *_var;
      } varParam;

      // ArrayParameterK  数组参数
      struct {
          struct treeNode *type_spec;
          struct treeNode *_var;
      } arrParam;
      
      // CompoundStatementK 复合语句
      struct {
          struct treeNode *local_decl;
          struct treeNode *stmt_list;
          //ExpType retType;
      } cmpdStmt;

      // ExpressionStatementK 表达式
      struct {
          struct treeNode *expr;
      } exprStmt;

      // SelectionStatementK if_then选择语句
      struct {
          struct treeNode *expr;
          struct treeNode *if_stmt;
          struct treeNode *else_stmt;
      } selectStmt;

      // IterationStatementK while循环语句
      struct {
          struct treeNode *expr;
          struct treeNode *loop_stmt;
      } iterStmt;

      // ReturnStatementK return返回表达式
      struct {
          struct treeNode *expr;
          //ExpType retType;
      } retStmt;

      // AssignExpressionK 赋值语句
      struct {
          struct treeNode *expr;
          struct treeNode *_var;
      } assignStmt;

      // ComparisonExpressionK 比较运算符
      struct {
          struct treeNode *lexpr;
          struct treeNode *op;
          struct treeNode *rexpr;
      } cmpExpr;

      // AdditiveExpressionK 加减运算表达式
      struct {
          struct treeNode *lexpr;
          struct treeNode *op;
          struct treeNode *rexpr;
      } addExpr;

      // MultiplicativeExpressionK 乘除运算表达式
      struct {
          struct treeNode *lexpr;
          struct treeNode *op;
          struct treeNode *rexpr;
      } multExpr;

      // ArrayK 数组引用 _var[arr_expr]
      struct {
          struct treeNode *_var;
          struct treeNode *arr_expr;
      } arr;

      // CallK 函数引用 _var(exper_list)
      struct {
          struct treeNode *_var;
          struct treeNode *expr_list;
      } call;

      // VariableK 标识符
      struct {
          char *ID;
      };

      // ConstantK 常量
      struct {
          int NUM;
      };

      // TokenTypeK 类别码
      struct {
          TokenType TOK;
      };
  } attr;
} TreeNode;

#endif
