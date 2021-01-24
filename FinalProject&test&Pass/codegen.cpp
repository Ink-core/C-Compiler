#include <iostream>
#include <vector>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/IR/TypeBuilder.h"
#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include "codegen.h"
#include "parser.hpp"

using namespace std;

class CodeGenContext;	//记录Module，内含Functions
Function *realmain;		//记录真正的main函数
Function *curfc;		//记录当前在哪个函数定义内
BasicBlock *curb;		//记录当前所在基本块
int manum = 0;			//源代码定义main函数的个数
IRBuilder<> builder(MyContext);//https://llvm.org/doxygen/classllvm_1_1IRBuilder.html#adc930237bf3c1779830384ea55dd4792,
//定义存在于MyContext的中间代码生成器
BasicBlock *curret;		//记录当前函数的exit块
std::vector<string> callList;
Value *calladdr;

Value* codegen(TreeNode *cur, CodeGenContext& context);


/* Compile the AST into a module */
void CodeGenContext::generateCode(CodeGenContext& context,TreeNode* root)
{
	std::cout << "start code generation...\n";
	codegen(root,context); /* emit bytecode for the toplevel block */
	/* Print the bytecode in a human-readable format 
	   to see if our program compiled properly
	 */
	std::cout << "Code is generated.\n";
	 module->dump();

	legacy::PassManager pm;
	pm.add(createPrintModulePass(outs()));
	pm.run(*module);
}

/* Executes the AST by running the main function */
GenericValue CodeGenContext::runCode() {
	if (manum != 1)			//检测源代码main函数有没有定义或者重定义
	{
		std::cout << "Main function ilegal!\n";
		exit(0);
	}
	std::cout << "Running code...\n";
	ExecutionEngine *ee = EngineBuilder( unique_ptr<Module>(module) ).create();
	ee->finalizeObject();
	vector<GenericValue> noargs;
	GenericValue v = ee->runFunction(realmain, noargs);		//这里记得要从真正的main函数开始执行代码
	std::cout << "Code was run.\n";
	return v;
}

//返回当前节点的数据类型
static Type *typeOf(TreeNode* type_spec) 
{
	if(type_spec->attr.TOK==INT)
		//return Type::getInt64Ty(MyContext);
		return IntegerType::get(MyContext , 64);
	else
		return Type::getVoidTy(MyContext);
}

/*              generate LLVM IR            */
Value* codegen(TreeNode *cur, CodeGenContext& context)
{
	if (cur == NULL)
	{
		return NULL;
	}
	switch (cur->nodeKind)		//对不同的节点要用不同的构造方法
	{
	case ProgramK://主函数
	{
		//之所以多定义一个ProgramK类型，就是因为他不同于复合语句，它允许全局变量和函数的交替定义
		TreeNode *head = cur->attr.Pro.cur;
		Value *last = NULL;
		while (head != NULL)
		{	//深度优先遍历
			codegen(head, context);
			head = head->sibling;
		}
		return last;

	}
	case VariableDeclarationK://变量声明
	//返回申请的内存地址 （Value *）
	{   
        std::cout<<"Variable Declaration : 【type】:";

        //             1. 【判断类型】

        string var_type;//内存分配用到的Twine类型
        if(cur->attr.varDecl.type_spec->attr.TOK==INT)
        {
            std::cout<<"int\n";
            var_type="int";
        }
        else
        {
            std::cout<<"void\n";
            var_type="void";
        }       
        string varname=cur->attr.varDecl._var->attr.ID;

        //      2.【静态语义检查：是否重定义】

        //在现有Module内函数栈内寻找变量，检查是否已经重定义
        if(context.isexist(varname))
        {
            std::cout<<"Variable ["<<varname<<"] already exist！！！\n";
            exit(0);
        }

        //          3.【分配内存】

        //在栈顶（现作用对象）基本块内分配空间
        //内存分配API:AllocaInst() [4/6] 
        //AllocaInst::AllocaInst（Type * Ty,unsigned AddrSpace,const Twine & Name,BasicBlock *InsertAtEnd )	
        //https://llvm.org/doxygen/classllvm_1_1AllocaInst.html#a51e656c962f10511a41edc8ac3507865
        AllocaInst *addr=  new AllocaInst(
			typeOf(cur->attr.varDecl.type_spec),
			0,
			var_type,
			curb
			);//这里只是分配内存，还没有将值传进去
		addr->setAlignment(4);

        //          4.【更新符号表】

        context.blocks.top()->locals[varname]=addr;//符号表内存放地址
        return addr;
	}
	
	case ArrayDeclarationK: //数组声明
	//返回申请的内存地址 （Value *）
	{
		std::cout<<"Array Declaration : 【type】:";

        //             1. 【判断类型】

        string var_type;//内存分配是用到的Twine类型
        if(cur->attr.arrDecl.type_spec->attr.TOK==INT)
        {
            std::cout<<"int\n";
            var_type="INT";
        }
        else
        {
            std::cout<<"void\n";
            var_type="void";
        }       
        string varname=cur->attr.arrDecl._var->attr.ID;

        //      2.【静态语义检查：是否重定义】

        //在现有Module内函数栈内寻找变量，检查是否已经重定义
        if(context.isexist(varname))
        {
            std::cout<<"Variable already exist！！！";
            exit(0);
        }

		//			3.【获得数组长度】

		//ConstantInt::get	(Type * Ty,uint64_t V,bool isSigned = false )
		//https://llvm.org/doxygen/classllvm_1_1ConstantInt.html#a9105541412dab869e18b3cceebfff07d	
		//IntegerType * Type::getInt64Ty(LLVMContext & C)
		//https://llvm.org/doxygen/classllvm_1_1Type.html#a30dd396c5b40cd86c1591872e574ccdf
		Value *e=ConstantInt::get(Type::getInt64Ty(MyContext),cur->attr.arrDecl._num->attr.NUM,true);

		 //          4.【分配空间】
		 
		//在栈顶（现作用对象）基本块内分配空间
        //内存分配API:AllocaInst() [2/6] 
        //AllocaInst::AllocaInst(Type * Ty,unsigned AddrSpace,Value * ArraySize,const Twine & Name,BasicBlock * InsertAtEnd )
        //https://llvm.org/doxygen/classllvm_1_1AllocaInst.html#a51e656c962f10511a41edc8ac3507865
		
       AllocaInst *addr= new AllocaInst(
		   typeOf(cur->attr.arrDecl.type_spec),
		   e,
		   varname,
		   curb);
	   addr->setAlignment(4);

        //          5.【更新符号表】

        context.blocks.top()->locals[varname]=addr;//符号表内存放地址
        return addr;
	}

	case FunctionDeclarationK:
	{				
		if(manum==1) return NULL;
		//					1【声明本函数】

		//函数定义的部分比较复杂，需要调用API并且传递很多数据。
		TreeNode *head;
		TreeNode *tmp;
		vector<TreeNode *> mknew;//树节点的副本
		mknew.clear();
		head = cur->attr.funcDecl.params;
		vector<Type *> argTypes;//参数列表
		head = cur->attr.funcDecl.params;
		//首先要将参数的类型记录下来，装在一个vector里，作为一个参数
		while (head != NULL)
		{
			std::cout << "Creating function parm type\n";
			argTypes.push_back(typeOf(head->attr.varParam.type_spec));
			//这里为每一个参数新建了一个副本，并且将其节点类型改为了变量声明
			//这是因为要在当前函数体内，为这些参数申请空间，这样这些参数在函数体内才是可用的
			tmp = new TreeNode;
			tmp->nodeKind = VariableDeclarationK;
			tmp->attr.varDecl.type_spec = head->attr.varParam.type_spec;
			tmp->attr.varDecl._var = head->attr.varParam._var;
			tmp->attr.TOK=head->attr.varParam._var->attr.TOK;
			mknew.push_back(tmp);
			head = head->sibling;
		}
		string fname = cur->attr.funcDecl._var->attr.ID;//函数名
		
		string paname;//参数名
		//创建类型
		FunctionType *ftype = FunctionType::get(
			typeOf(cur->attr.funcDecl.type_spec), 
			makeArrayRef(argTypes), 
			false);
		Function *function = Function::Create(
			ftype, GlobalValue::ExternalLinkage, 
			fname.c_str(), 
			context.module
			);//创建函数并加入Module
		context.push_function();//将新函数入栈
		context.blocks.top()->func=function;
		context.blocks.top()->fname=fname;
		if(fname=="main") 
		{
			manum++;
			std::cout<<"main number: "<<manum<<"\n";
			realmain=function;
		}
		
		
		
		//					2.【处理本函数参数内存分配问题】
		std::vector<AllocaInst *> paramaddrs;//参数内存位置向量
		curfc=function;//设置当前处理的函数
		BasicBlock *bblock = BasicBlock::Create(MyContext, "entry", function);//entry块添加进函数
		builder.SetInsertPoint(bblock);//设置指令插入点在函数入口的entry块
		curb=bblock;
		if(typeOf(cur->attr.funcDecl.type_spec)->isIntegerTy())
		{
			std::cout<<"【return int】\n";
			AllocaInst *retVal = new AllocaInst(typeOf(cur->attr.funcDecl.type_spec),0, "retval"+fname, bblock);
			context.blocks.top()->returnValue = retVal;//在entry块中为返回值分配了空间
		}	
		else
		{
			context.blocks.top()->returnValue = nullptr;
			std::cout<<"【return void】\n";
		}
		
		//接下来在entry块内进行内存的管理
		
		//参数命名
		Function::arg_iterator argsValues = function->arg_begin();//获取函数参数迭代器
		vector<TreeNode *>::iterator it;//参数树节点迭代器
		Function::arg_iterator pit = function->arg_begin();
		for (it = mknew.begin(); it != mknew.end(); it++) 
		{
			paname = (*it)->attr.varParam._var->attr.ID;//获得参数名
			//获得参数类型
			string var_type;//内存分配用到的Twine类型
			//分配内存&存入参数向量中
			AllocaInst *alloc = new AllocaInst(
				//typeOf((*it)->attr.varParam.type_spec),
				IntegerType::get(MyContext , 64),
				0, 
				var_type , 
				bblock);
			alloc->setAlignment(4);
			paramaddrs.push_back(alloc);
			context.locals()[paname]=alloc;//更新符号表
			std::cout << "Function parm value" << paname << '\n';
			//命名
			Value *argumentValue = &*argsValues++;
			argumentValue->setName(paname);
			argumentValue++;
			context.blocks.top()->locals[paname]=alloc;//入符号表
			//将实参的值存入形参的内存
			Value *pa= &(*(pit++));
			std::cout<<"use StoreInst here, IN param rem assign\n";
			StoreInst *st0 = new StoreInst(pa, alloc, false, bblock);
			delete(*it);//释放树副本空间
			*it = NULL;	
		}

		//				3【生成函数体IR】

		codegen(cur->attr.funcDecl.cmpd_stmt,context);
		BasicBlock *FR = BasicBlock::Create(MyContext, "terminator", function);
		builder.CreateBr(FR);
		builder.SetInsertPoint(FR);
		curb=FR;
		if(typeOf(cur->attr.funcDecl.type_spec)->isIntegerTy())
		{
			LoadInst *ld = new LoadInst(context.blocks.top()->returnValue, "", false, curb);
    		ld->setAlignment(4);
			std::cout<<"return int\n";
    		return ReturnInst::Create(MyContext, ld, curb);
		}
		else
		{
			std::cout<<"return void\n";
			return ReturnInst::Create(MyContext, nullptr, curb);
		}
	}
	case CompoundStatementK:
	{
		//			1.【local declaration】
		TreeNode* head = cur->attr.cmpdStmt.local_decl;
		while (head != NULL)
		{
			codegen(head, context);
			head = head->sibling;
		}
		//			2.【statement list】
		head = cur->attr.cmpdStmt.stmt_list;
		while (head != NULL)
		{
			codegen(head, context);
			head = head->sibling;
		}
		return NULL;
	}
	case ExpressionStatementK:
	{
		std::cout << "Creating code for expression\n";
		return codegen(cur->attr.exprStmt.expr,context);
	}
	case SelectionStatementK:
	{
		std::cout << "Creating code for IF ELSE\n";

		// 				1.【确定条件值】

		Function *function=curfc;
		Function::arg_iterator argsIT = function->arg_begin();
		/*Value *condval = codegen(cur->attr.selectStmt.expr, context);
		Type *condtype = condval->getType();
		//Type* llvm::Value::getType()	
		//https://llvm.org/doxygen/classllvm_1_1Value.html#a6393a2d4fe7e10b28a0dcc35f881567b
		if (condtype->isIntegerTy())
		{	//解决类型转换问题：https://stackoverflow.com/questions/47264133/llvm-ir-c-api-typecast-from-i1-to-i64-and-i64-to-i1
			//isIntegerTy() [1/2] True if this is an instance of IntegerType.
			//bool llvm::Type::isIntegerTy() const
			//https://llvm.org/doxygen/classllvm_1_1Type.html
			condval = builder.CreateIntCast(condval, Type::getInt64Ty(MyContext), true);//转为int64常数
			condval = builder.CreateICmpNE(condval, ConstantInt::get(Type::getInt64Ty(MyContext), 0, true));//转为bool类型
		}*/
		//  			2【创建IF THEN ELSE 控制流图】

		BasicBlock *ThenBB =BasicBlock::Create(MyContext, "then", curfc);
		BasicBlock *ElseBB = BasicBlock::Create(MyContext, "else",curfc);
		BasicBlock *MergeBB = BasicBlock::Create(MyContext, "merge",curfc);
		Value *cond=codegen(cur->attr.selectStmt.expr, context);
		Type *condty=cond->getType();
		BranchInst::Create(ThenBB, ElseBB, cond, curb);
		
		
		//builder.CreateCondBr(condval, ThenBB, ElseBB); //创造条件分支
		// Emit then value.
		builder.SetInsertPoint(ThenBB);
		curb=ThenBB;
		//在if_then block中会出现新的变量或数组声明，不符合文法中一个函数的声明部分
		//在函数体开头的位置，因此需要将thenblock作为一个作用域，为其在函数栈顶新开
		//空间，then的IR生成结束后再弹出栈顶，与书中chap7说明语句的翻译模式一致
		//context.pushBlock(ThenBB);
		std::cout<<"Then Start\n";
		Value *ThenV = codegen(cur->attr.selectStmt.if_stmt,context);
		std::cout<<"Then over\n";
		if (!ThenV) return nullptr;
		builder.CreateBr(MergeBB);
		//context.popBlock();
		//curfc->getBasicBlockList().push_back(ElseBB);
		curb=ElseBB;
		// Codegen of 'Then' can change the current block, update ThenBB for the PHI.
		//ThenBB = builder.GetInsertBlock();	
		// Emit else block.
		//curfc->getBasicBlockList().push_back(ElseBB);
		//https://llvm.org/doxygen/classllvm_1_1Function.html#a71918985bc82353ecbc009fcdd7ffa38
		builder.SetInsertPoint(ElseBB);
		//context.pushBlock(ElseBB);
		std::cout<<"Else Start\n";
		Value *ElseV = codegen(cur->attr.selectStmt.else_stmt,context);
		std::cout<<"Else over\n";
		if (!ElseV) return nullptr;
		builder.CreateBr(MergeBB);
		builder.SetInsertPoint(MergeBB);
		//context.popBlock();
		// codegen of 'Else' can change the current block, update ElseBB for the PHI.
		//ElseBB = builder.GetInsertBlock();

		// 					3.【处理Merge块】

 		//curfc->getBasicBlockList().push_back(MergeBB);
		curb=MergeBB;
		 //Merge块需要与IF前的基本块符号表（当前栈顶符号表）一致
		std::map<std::string, Value*> succeedtable = context.locals();//当前栈顶符号表的副本
		Value *succeedretTy = context.getCurrentReturnValue();
		//context.popBlock();//弹出IF前的基本块
		//context.pushBlock(MergeBB, succeedtable, succeedretTy);//压入Merge块
		builder.SetInsertPoint(MergeBB);
		std::cout<<"SelectStmt over\n";
		return succeedretTy;
		
	}
	case IterationStatementK:
	{	

		std::cout << "Creating code for WHILE\n";
		//wl块是while语句条件判断的单独一个基本块，因为需要被多次跳转到此
		BasicBlock * wl = BasicBlock::Create(MyContext, "wl", curfc);
		//while函数体
		BasicBlock * then = BasicBlock::Create(MyContext, "then", curfc);
		//while语句后的语句
		BasicBlock * merge = BasicBlock::Create(MyContext, "merge", curfc);
		
		builder.CreateBr(wl);
		curb=wl;
		builder.SetInsertPoint(wl);
		
		BranchInst::Create(then,merge, codegen(cur->attr.selectStmt.expr, context), curb);

		builder.SetInsertPoint(then);
		curb=then;
		codegen(cur->attr.iterStmt.loop_stmt, context);
		builder.CreateBr(wl);

		std::map<std::string, Value*> nowloc = context.locals();
		Value* nowret = context.getCurrentReturnValue();
		curb=merge;
		builder.SetInsertPoint(merge);
		return nowret;	
	}
	case ReturnStatementK:
	{

		std::cout << "Creating return code\n";
		StoreInst *st = new StoreInst(codegen(cur->attr.retStmt.expr,context),context.blocks.top()->returnValue,false,curb);
		//LoadInst *ld = new LoadInst(context.blocks.top()->returnValue, "", false, curb);
    	//ld->setAlignment(4);
		return st;
    	//return ReturnInst::Create(MyContext, ld, curb);
		
	}
	case AssignExpressionK:
	{
		std::cout<<"Create AssignExperssion\n";
		// 				1【获得右端值】

		Value * rhs=codegen(cur->attr.assignStmt.expr,context);
		
		// 				2【判断左端值的类型】

		if(cur->attr.assignStmt._var->nodeKind==VariableK)
		//变量类型：检查该变量是否已经被定义，若未定义则报错
		{
			string var_name=cur->attr.assignStmt._var->attr.ID;
			Value *ie=context.isexist(var_name);
			if(!ie)
			{
				std::cout<< "variable ["<<var_name<<"] has\'nt been declared\n";
				return NULL;
			}
			else
			{	//StoreInst[4/8]
				//https://llvm.org/doxygen/classllvm_1_1StoreInst.html#aa2a72f9a51b317f5b4ab8695adf59025
				//An instruction for storing to memory.
				//在变量声明的时候已经给它分配过内存了，现在通过isexist获得地址后存入值即可
				//	StoreInst::StoreInst	(	Value * 	Val,
				//								Value * 	Ptr,
				//								bool 	isVolatile,
				//								BasicBlock * 	InsertAtEnd )
				std::cout<<"use StoreInst here,hhhhhhh!!! in assign\n";
				StoreInst* st = new StoreInst( rhs,ie, false, curb);
				std::cout<<"over StoreInst here,hhhhhhh!!! in assign\n";
			}	
		}
		else
		//数组类型：先获得下标
		{
			string var_name=cur->attr.assignStmt._var->attr.arr._var->attr.ID;
			Value *ie=context.isexist(var_name);
			if(!ie)
			{
				std::cout<< "variable ["<<var_name<<"] has\'nt been declared\n";
				return NULL;
			}
			else
			{
				Value *e=codegen(cur->attr.assignStmt._var->attr.arr.arr_expr,context);//获得下标
				Value *pos= builder.CreateGEP(ie,e);//获得数组下标处对应的指针
				std::cout<<"use StoreInst here,hhhhhhh!!!in assign\n";
				StoreInst* st=new StoreInst(rhs,pos,false,curb);
				std::cout<<"over StoreInst here,hhhhhhh!!! in assign\n";
			}
		}
		return rhs;
	}
	case ComparisonExpressionK:
	{
		//关系运算语句，这个部分就是递归的调用左右两边式子的codegen，并且
		//调用相应API即可，但是一定要注意一点，API的两个参数必须类型一致
		//否则在编译时就会报错，导致无法执行
		std::cout << "Creating CMP operation\n";

		//				1.【获得左右表达式的值】

		Value *left = codegen(cur->attr.cmpExpr.lexpr, context);
		Value *right = codegen(cur->attr.cmpExpr.rexpr, context);

		//				2.【判断不等号的类型】
		//根据不等号的类型生成不同种类的IR
		switch (cur->attr.cmpExpr.op->attr.TOK)
		{
			//https://llvm.org/doxygen/classllvm_1_1IRBuilderBase.html#a6a2a72ba13b8ceb231afa29dca55cf62
			//Value* llvm::IRBuilderBase::CreateICmpSLT	(	Value * 	LHS,
			//												Value * 	RHS,
			//												const Twine & 	Name = "" )
		case LT:
			//return builder.CreateICmpSLT(left, right);
			{CmpInst *cmpRes = ICmpInst::Create(Instruction::ICmp,
        			ICmpInst::Predicate::ICMP_SLT, left, right, "cmp", curb);
			return cmpRes;}
		case LE:
			//return builder.CreateICmpSLE(left, right);
			{CmpInst *cmpRes = ICmpInst::Create(Instruction::ICmp,
        			ICmpInst::Predicate::ICMP_SLE, left, right, "cmp", curb);
			return cmpRes;}
		case GT:
			{CmpInst *cmpRes = ICmpInst::Create(Instruction::ICmp,
        			ICmpInst::Predicate::ICMP_SGT, left, right, "cmp", curb);
			return cmpRes;}
			//return builder.CreateICmpSGT(left, right);
		case GE:
			//return builder.CreateICmpSGE(left, right);
			{CmpInst *cmpRes = ICmpInst::Create(Instruction::ICmp,
        			ICmpInst::Predicate::ICMP_SGE, left, right, "cmp", curb);
			return cmpRes;}
		case EQ:
			//return builder.CreateICmpEQ(left, right);
			{CmpInst *cmpRes = ICmpInst::Create(Instruction::ICmp,
        			ICmpInst::Predicate::ICMP_EQ, left, right, "cmp", curb);
			return cmpRes;}
		case NE:
			//return builder.CreateICmpNE(left, right);
			{CmpInst *cmpRes = ICmpInst::Create(Instruction::ICmp,
        			ICmpInst::Predicate::ICMP_NE, left, right, "cmp", curb);
			return cmpRes;}
		default:
			std::cout << "CMP error!\n";
			exit(0);
		}
	}

	case MultiplicativeExpressionK:
	case AdditiveExpressionK:
	{
		fprintf(listing, "Creating MATH operation \n");
		Instruction::BinaryOps instr;
		switch (cur->attr.addExpr.op->attr.TOK) 
		{
		case PLUS: 	instr = Instruction::Add; goto math;
		case MINUS: 	instr = Instruction::Sub; goto math;
		case TIMES: 		instr = Instruction::Mul; goto math;
		case OVER: 		instr = Instruction::SDiv; goto math;

			/* TODO comparison */
		}

		return NULL;
	math:
		//Construct a binary instruction, given the opcode and the two operands.组装算符和操作数并自动生成IR
		//Also automatically insert this instruction to the end of the BasicBlock specified.
		//https://llvm.org/doxygen/classllvm_1_1BinaryOperator.html#a02ce9966395063ac501ecbc1623deda4
		//Create() [2/2]
		//BinaryOperator * BinaryOperator::Create	(	BinaryOps 	Op,
		//												Value * 	S1,
		//												Value * 	S2,
		//												const Twine & 	Name,
		//												BasicBlock * 	InsertAtEnd )	
		return BinaryOperator::Create(instr, codegen(cur->attr.addExpr.lexpr,context),
			codegen(cur->attr.addExpr.rexpr,context), "", curb);
	}
	case ArrayK:
	{
		//加载数组变量，比起简单变量，多一个数组下标的codegen
		std::cout << "Creating array identifier reference: " << cur->attr.arr._var->attr.ID << '\n';
		string name = cur->attr.arr._var->attr.ID;
		Value *arr = context.isexist(name);//数组指针的存储地址
		if (arr == NULL)//语义检查：定义后引用
		{
			std::cout << "Undeclared array: " << name << '\n';
			return NULL;
		}
		Value *pos = codegen(cur->attr.arr.arr_expr, context);//数组下标
		Value *ele = builder.CreateGEP(arr, pos);//数组下标处数据对饮的指针
		//An instruction for reading from memory.
		//http://formalverification.cs.utah.edu/llvm_doxy/2.9/classllvm_1_1LoadInst.html
		//LoadInst (Value *Ptr, const Twine &NameStr, bool isVolatile, BasicBlock *InsertAtEnd)
		return new LoadInst(ele, "", false, curb);
	}
	case CallK:
	{
		//				1【获取函数名，检查函数是否被定义】
		string fname = cur->attr.call._var->attr.ID;
		Function *function = context.module->getFunction(fname);
		if (function == NULL) 
		{
			std::cout << "No such function " << cur->attr.call._var->attr.ID << '\n';
			exit(0);
		}
		else
		{
			std::cout<<"Call "<<fname<<"\n";
		}
		
		//				2【函数被定义：构造函数的参数列表】
		TreeNode *head = cur->attr.call.expr_list;
		std::vector<Value*> args;
		int count=0;
		int realcount=0;
		while (head != NULL)
		{	
			count+=1;
			args.push_back(codegen(head,context));
			head = head->sibling;
		}
		for(auto arg = function->arg_begin(); arg != function->arg_end(); ++arg)
		{
			realcount++;
		} 
		if(count!=realcount)
		{
			std::cout<<"Wrong number of function parameter\n";
			exit(0);
		}
		ArrayRef<Value*>  argsRef(args);
		//				3【生成函数调用指令】

		 CallInst *call = CallInst::Create(function, makeArrayRef(args), "", curb);
		 return call;
	}
	case VariableK:
	{
		//加载变量
		std::cout << "Creating identifier reference: " << cur->attr.ID <<'\n';
		string name = cur->attr.ID;
		Value *arg = context.isexist(name);
		if (arg == NULL)
		{
			std::cout << "Undeclared variable: " << cur->attr.ID <<'\n';
			return NULL;
		}
		LoadInst *ld = new LoadInst(arg, "", false, curb);
		return ld;

	}
	case ConstantK:
	{
		//常量值，即从C语言中的一个int类型值，得到一个llvm ir中用的Value类型值
		std::cout << "Creating integer: " << cur->attr.NUM <<'\n';
		return ConstantInt::get(Type::getInt64Ty(MyContext), cur->attr.NUM, true);
	}
	default:
		return NULL;
	}
}


