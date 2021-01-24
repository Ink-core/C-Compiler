#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

//效果：
//我们的pass准确地找到了有关def被使用的具体位置（Instructions）。
//Value 类型也是可以直接打印出来的

namespace
{

    struct DefUse : public FunctionPass
    {
        static char ID; // Pass identification, replacement for typeid
        DefUse() : FunctionPass(ID) {}

        bool runOnFunction(Function &F) override
        {

            errs() << "Function name: ";
            errs() << F.getName() << '\n';

            //遍历函数中每一个基本块
            for (Function::iterator bb = F.begin(), e = F.end(); bb != e; bb++)
            {
                //遍历基本块中每一条指令
                for (BasicBlock::iterator i = bb->begin(), i2 = bb->end(); i != i2; i++)
                {
                    //dyn_cast<>
                    //https://llvm.org/doxygen/Casting_8h_source.html
                    Instruction *inst = dyn_cast<Instruction>(i); //指令 i
                    errs() << "\t" << *inst << endl;//Print
                    //嗯，就是这里

                    //遇到 add, Icmp 指令
                    if (inst->getOpcode() == Instruction::Add || inst->getOpcode() == Instruction::ICmp)
                    {
                        //对每一个使用者来说
                        //User
                        //https://llvm.org/doxygen/classllvm_1_1User.html
                        //
                        //https://blog.csdn.net/ioripolo/article/details/5401821
                        for (User *U : inst->users())
                        {
                            //如果这个指令存在
                            //这种用法很奇妙，是在官方文档里的标准用法
    
                            //if U 是 Instruction 类型的
                            if (Instruction *Inst = dyn_cast<Instruction>(U)) //Inst 的类型还可以用 auto 替代
                            //这种写法的好处是，把类型转换和判断非空合二为一了，就这么些
                            {
                                // Inst is an instruction use i, right! ! !
                                outs() << "OpCode " << inst->getOpcodeName() << " used in :: ";
                                outs() << *Inst << "\n";
                            }
                        }
                    }
                }
            }

            return false;
        }
    };
} // namespace

char DefUse::ID = 0;
static RegisterPass<DefUse> X("DefUse", "This is def-use Pass");
