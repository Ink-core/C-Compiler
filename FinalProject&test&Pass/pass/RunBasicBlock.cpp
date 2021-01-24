#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

//基本遍历
namespace
{

    struct RunBasicBlock : public FunctionPass
    {
        static char ID; // Pass identification, replacement for typeid
        RunBasicBlock() : FunctionPass(ID) {}

        bool runOnFunction(Function &F) override
        {
            errs() << "Function name: ";
            errs() << F.getName() << '\n';
            //遍历 Function 里面每一个 block
            for (Function::iterator bb = F.begin(), e = F.end(); bb != e; bb++)
            {
                errs() << "BasicBlock name = " << bb->getName() << "\n";
                errs() << "BasicBlock size = " << bb->size() << "\n\n";
                
                //遍历 block 里面每一条指令
                for (BasicBlock::iterator i = bb->begin(), i2 = bb->end(); i != i2; i++)
                {
                    outs() << "    " << *i << "\n";
                }
            }
            return false;
        }
    };
} // namespace

char RunBasicBlock::ID = 0;
static RegisterPass<RunBasicBlock> X("RunBasicBlock", "run every basicblocks inside a Function");
