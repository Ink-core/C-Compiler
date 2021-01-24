%{
#define YYPARSER /* distinguishes Yacc output from other code files */
#include "globals.h"
#include "ast.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode * //自定义yyval的类型YYSTYPE为Treenode*
static TreeNode * savedTree; /* stores syntax tree for later return ，最后的AST就在这里*/
//语法分析程序有yylex（bison与的flex交互通道）和yyerror（出错处理）
static int yylex(void);
int yyerror(char*);
%}

/*定义终结符与词法分析器中的return匹配*/
%token ID NUM
%token ELSE IF INT RETURN VOID WHILE
%token PLUS MINUS TIMES OVER
%token LT LE GT GE EQ NE
%token ASSIGN SEMI COMMA
%token LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE
%token ENDFILE ERROR
%token MAXIMUM_TOKEN

%nonassoc LOWER_ELSE
%nonassoc ELSE

%start program

%% /* Grammar for C- */

program
        : declaration_list
          { savedTree = makehead($1);}
          //生成抽象语法树的根节点 }
        ;

declaration_list
        : declaration_list declaration
          { $$ = addSibling($1, $2); }
        | declaration
          { $$ = $1; }
        ;

declaration
        : var_declaration
          { $$ = $1; }
        | fun_declaration
          { $$ = $1; }
        ;

var_declaration
        : type_specifier _id SEMI
          { $$ = newVariableDeclarationNode($1, $2); }
        | type_specifier _id LBRACK _num RBRACK SEMI
          { $$ = newArrayDeclarationNode($1, $2, $4); }
        ;

type_specifier
        : INT
          { $$ = newTokenTypeNode(INT); }
        | VOID
          { $$ = newTokenTypeNode(VOID); }

fun_declaration
        : type_specifier _id LPAREN params RPAREN compound_stmt
          { $$ = newFunctionDeclarationNode($1, $2, $4, $6); }
        ;

params
        : param_list
          { $$ = $1; }
        | VOID
          { $$ = NULL; }
        ;

param_list
        : param_list COMMA param
          { $$ = addSibling($1, $3); }
        | param
          { $$ = $1; }
        ;

param
        : type_specifier _id
          { $$ = newVariableParameterNode($1, $2); }
        | type_specifier _id LBRACK RBRACK
          { $$ = newArrayParameterNode($1, $2); }
        ;

compound_stmt
        : LBRACE local_declarations statement_list RBRACE
          { $$ = newCompoundStatementNode($2, $3); }
        ;

local_declarations
        : local_declarations var_declaration
          { $$ = addSibling($1, $2); }
        | /* empty */
          { $$ = NULL; }
        ;

statement_list
        : statement_list statement
          { $$ = addSibling($1, $2); }
        | /* empty */
          { $$ = NULL; }
        ;

statement
        : expression_stmt
          { $$ = $1; }
        | compound_stmt
          { $$ = $1; }
        | selection_stmt
          { $$ = $1; }
        | iteration_stmt
          { $$ = $1; }
        | return_stmt
          { $$ = $1; }
        ;

expression_stmt
        : expression SEMI
          { $$ = newExpressionStatementNode($1); }
        | SEMI
          { $$ = NULL; }
        ;

selection_stmt
        : IF LPAREN expression RPAREN statement
          { $$ = newSelectionStatementNode($3, $5, NULL); }
          %prec LOWER_ELSE
        | IF LPAREN expression RPAREN statement ELSE statement
          { $$ = newSelectionStatementNode($3, $5, $7); }
        ;

iteration_stmt
        : WHILE LPAREN expression RPAREN statement
          { $$ = newIterationStatementNode($3, $5); }
        ;

return_stmt
        : RETURN SEMI
          { $$ = newReturnStatementNode(NULL); }
        | RETURN expression SEMI
          { $$ = newReturnStatementNode($2); }
        ;

expression
        : var ASSIGN expression
         { $$ = newAssignExpressionNode($1, $3); }
        | simple_expression
         { $$ = $1; }
        ;

var
        : _id
          { $$ = $1; }
        | _id LBRACK expression RBRACK
          { $$ = newArrayNode($1, $3); }
        ;

simple_expression
        : additive_expression relop additive_expression
          { $$ = newComparisonExpressionNode($1, $2, $3); }
        | additive_expression
          { $$ = $1; }
        ;

relop
        : LT
          { $$ = newTokenTypeNode(LT); }
        | LE
          { $$ = newTokenTypeNode(LE); }
        | GT
          { $$ = newTokenTypeNode(GT); }
        | GE
          { $$ = newTokenTypeNode(GE); }
        | EQ
          { $$ = newTokenTypeNode(EQ); }
        | NE
          { $$ = newTokenTypeNode(NE); }
        ;

additive_expression
        : additive_expression addop term
          { $$ = newAdditiveExpressionNode($1, $2, $3); }
        | term
          { $$ = $1; }
        ;

addop
        : PLUS
          { $$ = newTokenTypeNode(PLUS); }
        | MINUS
          { $$ = newTokenTypeNode(MINUS); }

term
        : term mulop factor
          { $$ = newMultiplicativeExpressionNode($1, $2, $3); }
        | factor
          { $$ = $1; }
        ;

mulop
        : TIMES
          { $$ = newTokenTypeNode(TIMES); }
        | OVER
          { $$ = newTokenTypeNode(OVER); }
        ;

factor
        : LPAREN expression RPAREN
          { $$ = $2; }
        | var
          { $$ = $1; }
        | call
          { $$ = $1; }
        | _num
          { $$ = $1; }
        ;

call
        : _id LPAREN args RPAREN
          { $$ = newCallNode($1, $3); }
        ;

args
        : arg_list
          { $$ = $1; }
        | /* empty */
          { $$ = NULL; }
        ;

arg_list
        : arg_list COMMA expression
          { $$ = addSibling($1, $3); }
        | expression
          { $$ = $1; }
        ;

_id
        : ID
          { $$ = newVariableNode(tokenString); }
        ;

_num
        : NUM
          { $$ = newConstantNode(tokenString); }
        ;

%%

int yyerror(char * message)
{ 
    //在输出流文件中输出存放错误信息
    //根据官方文档，这里的message：For a syntax error, the string is normally "syntax error"
    //因此设置printtoken函数输出具体出错的token是什么
  	fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  	fprintf(listing, "Current token: ");
  	printToken(yychar,tokenString);//from util.cpp
  	return 0;
}

static int yylex(void)
{
  	TokenType tok = getToken();//得到词法分析程序返回的类别码
    //扫描到文件末尾 返回0
  	if (tok == ENDFILE) return 0;
    //出错，将出错信息写入输出流文件，返回0
  	if (tok == ERROR)
    {
      	fprintf(listing,"Lexical analyze error at line %d\n",lineno);
      	fprintf(listing,"Current token: %s",tokenString);
      	return 0;
    }
    //无错误或文件结束的正常情况直接返回类别码
  	return tok;
}

TreeNode * parse(void)
//main通过include<parse.h>中对该函数的声明来调用该函数
{   //调用语法分析程序，自下而上分析同时生成AST
  	yyparse();
    //返回AST
  	return savedTree;
}

