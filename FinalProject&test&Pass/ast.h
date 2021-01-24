#ifndef _UTIL_H_
#define _UTIL_H_
#include <string.h>
//globals.h中各类节点的生成函数
TreeNode* addSibling(TreeNode *, TreeNode *);
TreeNode* makehead(TreeNode *);
TreeNode* allocateTreeNode(void);
int TokenTypeChecker(TokenType);
int NodeKindChecker(TreeNode *, NodeKind);

TreeNode *newVariableDeclarationNode(TreeNode *, TreeNode *);
TreeNode *newArrayDeclarationNode(TreeNode *, TreeNode *, TreeNode *);
TreeNode *newFunctionDeclarationNode(TreeNode *, TreeNode *, TreeNode *, TreeNode *);

TreeNode *newVariableParameterNode(TreeNode *, TreeNode *);
TreeNode *newArrayParameterNode(TreeNode *, TreeNode *);

TreeNode *newCompoundStatementNode(TreeNode *, TreeNode *);
TreeNode *newExpressionStatementNode(TreeNode *);
TreeNode *newSelectionStatementNode(TreeNode *, TreeNode *, TreeNode *);
TreeNode *newIterationStatementNode(TreeNode *, TreeNode *);
TreeNode *newReturnStatementNode(TreeNode *);

TreeNode *newAssignExpressionNode(TreeNode *, TreeNode *);
TreeNode *newComparisonExpressionNode(TreeNode *, TreeNode *, TreeNode *);
TreeNode *newAdditiveExpressionNode(TreeNode *, TreeNode *, TreeNode *);
TreeNode *newMultiplicativeExpressionNode(TreeNode *, TreeNode *, TreeNode *);

TreeNode *newVariableNode(char *);
TreeNode *newArrayNode(TreeNode *, TreeNode *);
TreeNode *newCallNode(TreeNode *, TreeNode *);
TreeNode *newConstantNode(char *);
TreeNode *newTokenTypeNode(TokenType);

/* Procedure printToken prints a token 
 * and its lexeme to the listing file
 * 将token的值和类别码输出到listing中
 */
void printToken(TokenType, const char*);

/* Function copyString allocates and makes a new
 * copy of an existing string
 * 复制字符串
 */
char* copyString(char*);

/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 * 以缩进的形式表示语法树
 */
void printTree(TreeNode*,int,int);

#endif
