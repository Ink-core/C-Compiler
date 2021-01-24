
#include "globals.h"
#include "ast.h"

static const char * const nodeName[] = {
    "ErrorK",
    "VariableDeclarationK",
    "ArrayDeclarationK",
    "FunctionDeclarationK",
    "VariableParameterK",
    "ArrayParameterK",

    "CompoundStatementK",

    "ExpressionStatementK",
    "SelectionStatementK",
    "IterationStatementK",
    "ReturnStatementK",
    "AssignExpressionK",

    "ComparisonExpressionK",
    "AdditiveExpressionK",
    "MultiplicativeExpressionK",

    "VariableK",
    "ArrayK",
    "CallK",

    "ConstantK",
    "TokenTypeK"
};

/* Procedure printToken prints a token 
 * and its lexeme to the listing file
 */

void printToken(TokenType token, const char* tokenString)
/*输出token类型，值（字符串数组形式）到文件listing中*/
{
  switch (token)
  /*输出到yyout的文件指针listing*/
    {
    case ENDFILE: fprintf(listing,"EOF\n"); break;
    case ERROR:   fprintf(listing,
                          "ERROR\t\t\t%s\n",
                          tokenString); break;

    case ID:      fprintf(listing,
                          "ID\t\t\t%s\n",
                          tokenString); break;
    case NUM:     fprintf(listing,
                          "NUM\t\t\t%s\n",
                          tokenString); break;

    case ELSE:    fprintf(listing,
                          "ELSE\t\t\t%s\n",
                          tokenString); break;
    case IF:      fprintf(listing,
                          "IF\t\t\t%s\n",
                          tokenString); break;
    case INT:     fprintf(listing,
                          "INT\t\t\t%s\n",
                          tokenString); break;
    case RETURN:  fprintf(listing,
                          "RETURN\t\t\t%s\n",
                          tokenString); break;
    case VOID:    fprintf(listing,
                          "VOID\t\t\t%s\n",
                          tokenString); break;
    case WHILE:   fprintf(listing,
                          "WHILE\t\t\t%s\n",
                          tokenString); break;

    case PLUS:    fprintf(listing,
                          "PLUS\t\t\t%s\n",
                          tokenString); break;
    case MINUS:   fprintf(listing,
                          "MINUS\t\t\t%s\n",
                          tokenString); break;
    case TIMES:   fprintf(listing,
                          "TIMES\t\t\t%s\n",
                          tokenString); break;
    case OVER:    fprintf(listing,
                          "OVER\t\t\t%s\n",
                          tokenString); break;

    case LT:      fprintf(listing,
                          "<\t\t\t%s\n",
                          tokenString); break;
    case LE:      fprintf(listing,
                          "<=\t\t\t%s\n",
                          tokenString); break;
    case GT:      fprintf(listing,
                          ">\t\t\t%s\n",
                          tokenString); break;
    case GE:      fprintf(listing,
                          ">=\t\t\t%s\n",
                          tokenString); break;
    case EQ:      fprintf(listing,
                          "==\t\t\t%s\n",
                          tokenString); break;
    case NE:      fprintf(listing,
                          "!=\t\t\t%s\n",
                          tokenString); break;

    case ASSIGN:  fprintf(listing,
                          "=\t\t\t%s\n",
                          tokenString); break;
    case SEMI:    fprintf(listing,
                          ";\t\t\t%s\n",
                          tokenString); break;
    case COMMA:   fprintf(listing,
                          ",\t\t\t%s\n",
                          tokenString); break;

    case LPAREN:   fprintf(listing,
                          "(\t\t\t%s\n",
                          tokenString); break;
    case RPAREN:   fprintf(listing,
                          ")\t\t\t%s\n",
                          tokenString); break;
    case LBRACK:   fprintf(listing,
                          "[\t\t\t%s\n",
                          tokenString); break;
    case RBRACK:   fprintf(listing,
                          "]\t\t\t%s\n",
                          tokenString); break;
    case LBRACE:   fprintf(listing,
                          "{\t\t\t%s\n",
                          tokenString); break;
    case RBRACE:   fprintf(listing,
                          "}\t\t\t%s\n",
                          tokenString); break;

    default:       /* should never happen */
           fprintf(listing,"error!!!!"); 
  }
}

TreeNode* makehead(TreeNode *cur)
//生成ProgrammeK节点
{
	TreeNode *t = new TreeNode;
	if (t == NULL)
    {
      fprintf(listing,
              "Out of memory error at line %d\n",
              lineno);
    }
  	else
    {
      t->nodeKind=ProgramK;
      t->attr.Pro.cur=cur;
    }
  return t;
}

TreeNode* addSibling(TreeNode *origin, TreeNode *follow)
//为origin最右侧添加兄弟节点follow
{
  if (origin != NULL) {
    TreeNode *t = origin;
    while (t->sibling != NULL) t = t->sibling;
    t->sibling = follow;
  }
  else {
    origin = follow;
  }
  return origin;
}

TreeNode* allocateTreeNode(void)
//生成空的树节点,初始化兄弟为空
{
  TreeNode *t = new TreeNode;
  if (t == NULL)
    {
      fprintf(listing,
              "Out of memory error at line %d\n",
              lineno);
    }
  else
    {
      t->sibling = NULL;
      t->lineno = lineno;
    }

  return t;
}

TreeNode* newVariableDeclarationNode(TreeNode *type_specifier,TreeNode *_var)
//变量声明类型AST节点
{
  TreeNode * t = allocateTreeNode();//建立空节点
  if (t != NULL)
    {
      t->nodeKind = VariableDeclarationK;
      t->attr.varDecl.type_spec = type_specifier;
      t->attr.varDecl._var = _var;
    }

  return t;
}

TreeNode*
newArrayDeclarationNode(TreeNode *type_specifier,
                        TreeNode *_var,
                        TreeNode *_num)
//数组声明类型AST节点
{
  TreeNode * t = allocateTreeNode();
  if (t != NULL)
    {
      t->nodeKind = ArrayDeclarationK;
      t->attr.arrDecl.type_spec = type_specifier;
      t->attr.arrDecl._var = _var;
      t->attr.arrDecl._num = _num;
    }

  return t;
}

TreeNode*
newFunctionDeclarationNode(TreeNode *type_specifier,
                           TreeNode *_var,
                           TreeNode *params,
                           TreeNode *compound_stmt)
//函数声明类型AST节点
{
  TreeNode * t = allocateTreeNode();
  if (t != NULL)
    {
      t->nodeKind = FunctionDeclarationK;
      t->attr.funcDecl.type_spec = type_specifier;
      t->attr.funcDecl._var = _var;
      t->attr.funcDecl.params = params;
      t->attr.funcDecl.cmpd_stmt = compound_stmt;
    }

  return t;
}

TreeNode*
newVariableParameterNode(TreeNode *type_specifier,
                         TreeNode *_var)
//参数变量类型AST节点                    
{
  TreeNode * t = allocateTreeNode();
  if (t != NULL)
    {
      t->nodeKind = VariableParameterK;
      t->attr.varParam.type_spec = type_specifier;
      t->attr.varParam._var = _var;
    }

  return t;
}

TreeNode*
newArrayParameterNode(TreeNode *type_specifier,
                      TreeNode *_var)
//数组参数类型AST节点（What the fuck is this？）
{
  TreeNode * t = allocateTreeNode();
  if (t != NULL)
    {
      t->nodeKind = ArrayParameterK;
      t->attr.arrParam.type_spec = type_specifier;
      t->attr.arrParam._var = _var;
    }

  return t;
}

TreeNode*
newCompoundStatementNode(TreeNode *local_declarations, // nullable
                         TreeNode *statement_list) // nullable
//复合语句类型AST节点
{
  TreeNode * t = allocateTreeNode();
  if (t != NULL)
    {
      t->nodeKind = CompoundStatementK;
      t->attr.cmpdStmt.local_decl = local_declarations;
      t->attr.cmpdStmt.stmt_list = statement_list;
    }

  return t;
}

TreeNode*
newExpressionStatementNode(TreeNode *expression) // nullable
//表达式类型AST节点
{
  TreeNode * t = allocateTreeNode();
  if (t != NULL)
    {
      t->nodeKind = ExpressionStatementK;
      t->attr.exprStmt.expr = expression;
    }

  return t;
}

TreeNode*
newSelectionStatementNode(TreeNode *expression, // nullable
                          TreeNode *if_statement,
                          TreeNode *else_statement) // nullable
//选择结构类型AST节点
{
  TreeNode * t = allocateTreeNode();
  if (t != NULL)
    {
      t->nodeKind = SelectionStatementK;
      t->attr.selectStmt.expr = expression;
      t->attr.selectStmt.if_stmt = if_statement;
      t->attr.selectStmt.else_stmt = else_statement;
    }

  return t;
}

TreeNode*
newIterationStatementNode(TreeNode *expression,
                          TreeNode *statement) // nullable
//迭代（循环）类型AST节点
{
  TreeNode * t = allocateTreeNode();
  if (t != NULL)
    {
      t->nodeKind = IterationStatementK;
      t->attr.iterStmt.expr = expression;
      t->attr.iterStmt.loop_stmt = statement;
    }

  return t;
}

TreeNode*
newReturnStatementNode(TreeNode *expression) // nullable
//返回类型AST节点
{
  TreeNode * t = allocateTreeNode();
  if (t != NULL)
    {
      t->nodeKind = ReturnStatementK;
      t->attr.retStmt.expr = expression;
    }

  return t;
}

TreeNode*
newAssignExpressionNode(TreeNode *var,
                        TreeNode *expression)
//赋值语句类型AST节点
{
  TreeNode * t = allocateTreeNode();
  if (t != NULL)
    {
      t->nodeKind = AssignExpressionK;
      t->attr.assignStmt._var = var;
      t->attr.assignStmt.expr = expression;
    }

  return t;
}

TreeNode*
newComparisonExpressionNode(TreeNode *left_expression,
                            TreeNode *relop,
                            TreeNode *right_expression)
//比较运算类型AST节点
{
  TreeNode * t = allocateTreeNode();
  if (t != NULL)
    {
      t->nodeKind = ComparisonExpressionK;
      t->attr.cmpExpr.lexpr = left_expression;
      t->attr.cmpExpr.op = relop;
      t->attr.cmpExpr.rexpr = right_expression;
    }

  return t;
}

TreeNode*
newAdditiveExpressionNode(TreeNode *left_expression,
                          TreeNode *addop,
                          TreeNode *right_expression)
//表达式加减运算类型AST节点
{
  TreeNode * t = allocateTreeNode();
  if (t != NULL)
    {
      t->nodeKind = AdditiveExpressionK;
      t->attr.addExpr.lexpr = left_expression;
      t->attr.addExpr.op = addop;
      t->attr.addExpr.rexpr = right_expression;
    }

  return t;
}

TreeNode*
newMultiplicativeExpressionNode(TreeNode *left_expression,
                                TreeNode *mulop,
                                TreeNode *right_expression)
//表达式乘除运算类型AST节点
{
  TreeNode * t = allocateTreeNode();
  if (t != NULL)
    {
      t->nodeKind = MultiplicativeExpressionK;
      t->attr.multExpr.lexpr = left_expression;
      t->attr.multExpr.op = mulop;
      t->attr.multExpr.rexpr = right_expression;
    }

  return t;
}

TreeNode *
newArrayNode(TreeNode *_var,
             TreeNode *expression)
//数组引用类型AST节点
{
  TreeNode * t = allocateTreeNode();
  if (t != NULL)
    {
      t->nodeKind = ArrayK;
      t->attr.arr._var = _var;
      t->attr.arr.arr_expr = expression;
    }

  return t;
}

TreeNode*
newCallNode(TreeNode *_var,
            TreeNode *args) // nullable
//函数调用节点
{
  TreeNode * t = allocateTreeNode();
  if (t != NULL)
    {
      t->nodeKind = CallK;
      t->attr.call._var = _var;
      t->attr.call.expr_list = args;
    }

  return t;
}

TreeNode *
newVariableNode(char *_ID)
//标识符作为叶子节点
{
  TreeNode * t = allocateTreeNode();
  if (t != NULL)
    {
      	t->nodeKind = VariableK;
      	t->attr.ID = copyString(_ID);
    }
  return t;
}

TreeNode *
newConstantNode(char *_NUM)
//常数叶子节点
{
  TreeNode * t = allocateTreeNode();
  if (t != NULL)
    {
      t->nodeKind = ConstantK;
      t->attr.NUM = atoi(_NUM);
    }

  return t;
}

TreeNode *
newTokenTypeNode(TokenType token)
//类别码叶子节点
{
  TreeNode * t = allocateTreeNode();
  if (t != NULL)
    {
      t->nodeKind = TokenTypeK;
      t->attr.TOK = token;
    }

  return t;

}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char*
copyString(char * s)
{
  size_t n;
  char * t;
  if (s==NULL) return NULL;
  n = strlen(s)+1;
  t = new char[n];
  strcpy(t,s);
  return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */

/* macros to increase/decrease indentation */


/* printSpaces indents by printing spaces */
static void printSpaces(int cnt)
{
  int i;
  for (i=0; i<cnt; i++) fprintf(listing," ");
}

/* operatorString returns string of operator */
static const char * operatorString(TokenType op)
{
  if(op == INT) return "int";
  if(op == VOID) return "void";

  if(op == PLUS) return "+";
  if(op == MINUS) return "-";
  if(op == TIMES) return "*";
  if(op == OVER) return "/";

  if(op == LT) return "<";
  if(op == LE) return "<=";
  if(op == GT) return ">";
  if(op == GE) return ">=";
  if(op == EQ) return "==";
  if(op == NE) return "!=";

  return "";
}


/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 */
void printTree(TreeNode* tree,int cnt,int cl)
{
  if (tree == NULL) 
  	return;
  for (; tree != NULL; tree = tree->sibling)
    {
      switch (tree->nodeKind)
        {
        case ErrorK:
        	printSpaces(cnt);
          	fprintf(listing,"[DEBUG] ErrorK at printTree\n");
          	break;
		
		case ProgramK:
			fprintf(listing,"starting\n");
			printTree(tree->attr.Pro.cur,cnt,1);
			fprintf(listing,"ended\n");
			break;
		
        case VariableDeclarationK:
        	printTree(tree->attr.varDecl.type_spec,cnt,0);
          	printTree(tree->attr.varDecl._var,2,1);
          	break;

        case ArrayDeclarationK:
          	printTree(tree->attr.arrDecl.type_spec,cnt,0);
          	printTree(tree->attr.arrDecl._var,2,0);
          	fprintf(listing,"[");
          	printTree(tree->attr.arrDecl._num,0,0);
          	fprintf(listing,"]\n");
          	break;

        case FunctionDeclarationK:
          	printTree(tree->attr.funcDecl.type_spec,cnt,0);
          	printTree(tree->attr.funcDecl._var,2,1);
          	printTree(tree->attr.funcDecl.params,cnt+2,1);
          	printTree(tree->attr.funcDecl.cmpd_stmt,cnt+2,1);
          	break;

        case VariableParameterK:
          	printTree(tree->attr.varParam.type_spec,cnt,0);
          	fprintf(listing,"  (");
          	printTree(tree->attr.varParam._var,0,0);
          	fprintf(listing,")\n");
          break;

        case ArrayParameterK:
          printTree(tree->attr.arrParam.type_spec,cnt,0);
          fprintf(listing,"  (");
          printTree(tree->attr.arrParam._var,0,0);
          fprintf(listing,"[])\n");
          break;

        case CompoundStatementK:
          	printTree(tree->attr.cmpdStmt.local_decl,cnt,1);
          	printTree(tree->attr.cmpdStmt.stmt_list,cnt,1);
          	break;

        case ExpressionStatementK:
          	printTree(tree->attr.exprStmt.expr,cnt,1);
          	break;

        case SelectionStatementK:
        	printSpaces(cnt);
			fprintf(listing,"Selection\n");
          	printTree(tree->attr.selectStmt.expr,cnt+2,1);
          	printTree(tree->attr.selectStmt.if_stmt,cnt+2,1);
          	printTree(tree->attr.selectStmt.else_stmt,cnt+2,1);
          	break;

        case IterationStatementK:
          	printSpaces(cnt);
			fprintf(listing,"While\n");
          	printTree(tree->attr.iterStmt.expr,cnt+2,1);
          	printTree(tree->attr.iterStmt.loop_stmt,cnt+2,1);
          break;

        case ReturnStatementK:
          	printSpaces(cnt);
			fprintf(listing,"Return\n");
         	printTree(tree->attr.retStmt.expr,cnt+2,1);
         	break;

        case AssignExpressionK:
          	printSpaces(cnt);
          	fprintf(listing,"=\n");
          	printTree(tree->attr.assignStmt._var,cnt+2,1);
          	printTree(tree->attr.assignStmt.expr,cnt+2,1);
          	break;

        case ComparisonExpressionK:
          	printTree(tree->attr.cmpExpr.op,cnt,1);
          	printTree(tree->attr.cmpExpr.lexpr,cnt+2,1);
          	printTree(tree->attr.cmpExpr.rexpr,cnt+2,1);
          	break;

        case AdditiveExpressionK:
          	printTree(tree->attr.addExpr.op,cnt,1);
         	printTree(tree->attr.addExpr.lexpr,cnt+2,1);
          	printTree(tree->attr.addExpr.rexpr,cnt+2,1);
          	break;

        case MultiplicativeExpressionK:
          	printTree(tree->attr.multExpr.op,cnt,1);
          	printTree(tree->attr.multExpr.lexpr,cnt+2,1);
          	printTree(tree->attr.multExpr.rexpr,cnt+2,1);
          	break;

        case VariableK:
        	printSpaces(cnt);
          	fprintf(listing,"%s", tree->attr.ID);
          	if(cl)
          		fprintf(listing,"\n");
          	break;

        case ArrayK:
        	printSpaces(cnt);
        	fprintf(listing,"Array_ele\n");
          	printTree(tree->attr.arr._var,cnt+2,1);
          	printTree(tree->attr.arr.arr_expr,cnt+2,1);
          	break;

        case CallK:
        	printSpaces(cnt);
          	fprintf(listing,"Call\n");
          	printTree(tree->attr.call._var,cnt+2,1);
          	printTree(tree->attr.call.expr_list,cnt+4,1);
          	break;

        case ConstantK:
        	printSpaces(cnt);
          	fprintf(listing,"%d", tree->attr.NUM);
          	if(cl)
          		fprintf(listing,"\n");
          	break;

        case TokenTypeK:
        	printSpaces(cnt);
        	fprintf(listing,"%s",operatorString(tree->attr.TOK));
        	if(cl)
          		fprintf(listing,"\n");
          	break;

        default:
        	printSpaces(cnt);
          	fprintf(listing,"[DEBUG] No such nodeKind\n");
        }
    }
}

