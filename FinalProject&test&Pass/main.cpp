#include <iostream>
#include "codegen.h"
#include "ast.h"
#include "scan.h"
#include "parse.h"

int lineno = 0;
FILE * source;
FILE * listing;

using namespace std;

//extern int yyparse();
TreeNode* syntaxTree;

void createCoreFunctions(CodeGenContext& context);

int main(int argc, char **argv)
{
	listing = stdout; 	  //�������Ļ���ɸĳ��ļ� 
	char pgm[120];
	cout << "Please input file name\n";
	cin >> pgm;
	source = fopen(pgm, "r");
	if (source == NULL)
	{
		fprintf(listing, "File %s not found\n", pgm);
		exit(0);
	}

	syntaxTree = parse();   //����д��cm.y���parse()�������й����﷨������ 
	fprintf(listing, "**********\nSyntax tree:\n**********\n");
	printTree(syntaxTree, 0, 1);
	fclose(source);

	if (!syntaxTree)
	{
		fprintf(listing, "Syntax error!\n");
		exit(0);
	}
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	InitializeNativeTargetAsmParser();
	CodeGenContext context;
	createCoreFunctions(context);
	context.generateCode(context,syntaxTree);
	context.runCode();

	return 0;
}


