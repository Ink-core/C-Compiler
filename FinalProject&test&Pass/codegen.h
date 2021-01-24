#include <stack>
#include <typeinfo>
#include "globals.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Bitcode/BitstreamReader.h>
#include <llvm/Bitcode/BitstreamWriter.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/raw_ostream.h>

//https://blog.csdn.net/haoel/article/details/4789364
//构造程序的基本层次如下
//LLVM中的构造层次：Context->Module->Function->Blocks->Instruction Builder
//其中函数采用栈式管理，与chap7说明语句的翻译模式的设定相同

using namespace llvm;

static LLVMContext MyContext;//定义最高层次Context

class CodeGenBlock {// 代码生成块：类似与第三层次Function,内部包含许多BasicBlock
public:
    //BasicBlock *block;//小的基本块指针
	Function *func;//函数
	string fname;	//函数名
    Value *returnValue;//返回值类型
    std::map<std::string, Value*> locals;//函数内的本地标识符符号表
};

class CodeGenContext //类似于第二层Module，内含许多function
{
	public:
    std::stack<CodeGenBlock *> blocks;//栈式管理函数
	Function *mainFunction;//主函数
    Module *module;//建立第二层次Moudle
	CodeGenContext() { module = new Module("main", MyContext); }//生成函数：在Myocntext中添加Module

	void generateCode(CodeGenContext& context,TreeNode* root);//遍历抽象语法树形成Module，在cpp中有定义
    GenericValue runCode();//运行主函数以执行抽象语法树，在cpp中有定义
	Value* getCurrentReturnValue() { return blocks.top()->returnValue; }//返回栈顶函数返回值类型
	std::map<std::string, Value*>& locals() { return blocks.top()->locals; }//返回栈顶函数的子符号表
	void push_function(){blocks.push(new CodeGenBlock());};//新建函数
	void popBlock() { CodeGenBlock *top = blocks.top(); blocks.pop(); delete top; }//删除栈顶函数
	CodeGenBlock *findfunction(string name)//寻找函数
	{
		CodeGenBlock* tmp;
		std::stack<CodeGenBlock *> help;
		CodeGenBlock *ret = NULL;
		while (!blocks.empty())
		{
			tmp = blocks.top();
			blocks.pop();
			help.push(tmp);
			if (tmp->fname == name)
			{
				ret = tmp;
				break;
			}
		}
		while (!help.empty())
		{
			tmp = help.top();
			help.pop();
			blocks.push(tmp);
		}
		return ret;
	}
	Value* isexist(string name)
	//在符号表中查找是否含有特定名字的标识符
	{
		CodeGenBlock* tmp;
		std::stack<CodeGenBlock *> help;
		Value *ret = NULL;
		while (!blocks.empty())
		{
			tmp = blocks.top();
			blocks.pop();
			help.push(tmp);
			if (tmp->locals.find(name) != tmp->locals.end())
			{
				ret = tmp->locals[name];
				break;
			}
		}
		while (!help.empty())
		{
			tmp = help.top();
			help.pop();
			blocks.push(tmp);
		}
		return ret;
	}
    
    
    //BasicBlock *currentBlock() { return blocks.top()->block; }//返回栈顶函数的基本块指针
    //void pushBlock(BasicBlock *block) { blocks.push(new CodeGenBlock()); blocks.top()->returnValue = NULL; blocks.top()->block = block; }
	//栈顶添加新函数，设定返回值为Null，基本块为指定基本块
	//void pushBlock(BasicBlock *block, std::map<std::string, Value*>& locals, Value* returnValue) 
	//{
	//	blocks.push(new CodeGenBlock());
	//	blocks.top()->returnValue = returnValue;
		//blocks.top()->block = block;
		//blocks.top()->locals = locals;
	//}
	//删除函数栈顶函数，类似于第七章说明语句的翻译模式中建立嵌套过程符号表的翻译模式
    //void popBlock() { CodeGenBlock *top = blocks.top(); blocks.pop(); delete top; }
	//将函数栈中各个函数的返回值类型都设定为value
   /* void setCurrentReturnValue(Value *value) 
	{
		CodeGenBlock* tmp;
		std::stack<CodeGenBlock *> help;
		while (!blocks.empty())
		{
			tmp = blocks.top();
			blocks.pop();
			tmp->returnValue = value;
			help.push(tmp);
		}
		while (!help.empty())
		{
			tmp = help.top();
			help.pop();
			blocks.push(tmp);
		}
	}
	void setmain(Function* fc) { mainFunction = fc; }//设定主函数
	*/
};


