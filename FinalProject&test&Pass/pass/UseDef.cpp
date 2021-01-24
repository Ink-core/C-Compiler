#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace
{

    struct UseDef : public FunctionPass
    {
        static char ID; // Pass identification, replacement for typeid
        UseDef() : FunctionPass(ID) {}

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
                    Instruction *inst = dyn_cast<Instruction>(i);
                    if (inst->getOpcode() == Instruction::Add)
                    {
                        //输出指令 inst 的所有操作数
                        for (Use &U : inst->operands())
                        {
                            Value *v = U.get();
                            //这里有输出 Value 类型的例子
                            outs() << *v;
                        }
                        outs() << endl;//
                    }
                }
            }
            return false;
        }
    };
} // namespace

char UseDef::ID = 0;
static RegisterPass<UseDef> X("UseDef", "This is use-def Pass");