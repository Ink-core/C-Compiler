//我要写循环优化，课堂上就只会这个
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/IR/CFG.h"
#include <set>
//差什么再加上去
using namespace llvm;



//匿名命名空间
namespace
{
    struct Loop_me : public LoopPass
    {
        /* data */
        static char ID;

        Loop_me() : LoopPass(ID)
        {
            //
        }

        virtual bool runOnLoop(Loop *, LPPassManager &LPM) override
        {
            bool changed = false;

        }
    };
    
}

char Loop_me::ID = 0;

// 注册pass，这个pass可能会改变CFG，所以将第三个参数设为true
static RegisterPass<Loop_me> X("loop_me", "loop optimition pass", true, false);


