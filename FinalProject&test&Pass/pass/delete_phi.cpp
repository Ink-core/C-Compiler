#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/IR/CFG.h"
#include <set>

using namespace llvm;
​
// 这个就这样子吧

namespace
{
    struct DeadBlock : public FunctionPass
    {
        static char ID;
        ​
        DeadBlock() : FunctionPass(ID) //构造函数
        {
        }

        virtual bool runOnFunction(llvm::Function &F) override
        {
            // pass的入口
            // 下一小节，我们将在这里添加代码
            bool changed = false;
            std::set<BasicBlock *> visitedSet;     //访问过的基本块
            std::set<BasicBlock *> unreachableSet; //到达不了的基本块
            // 从EntryBlock开始深度优先遍历整个函数内可以访问的BaseBlock
            // 将已被访问过的BaseBlock存放在visitedSet中
            //https://llvm.org/doxygen/DepthFirstIterator_8h_source.html (line:239)
            auto i = df_ext_begin<BasicBlock*, std::set<BasicBlock*> >(&F.getEntryBlock(), visitedSet);
            auto e = df_ext_end<BasicBlock*, std::set<BasicBlock*> >(&F.getEntryBlock(), visitedSet);
            for (; i != e; i++)
            {
            }
            // 这个空循环貌似什么都没有做，但是因为 df_ext_begin() 的功能已经把遍历到的节点塞进 visitedSet 里面去了

            // 遍历函数内所有BaseBlock，将不在vistitedSet中的BaseBlock添加到unreachableSet中
            for (BasicBlock &BB : F)
            {
                // = .end() 说明不存在
                if (visitedSet.find(&BB) == visitedSet.end())
                {
                    unreachableSet.insert(&BB);
                }
            }
            // 标记目标函数是否会被修改
            if (!unreachableSet.empty())
            {
                changed = true;
            }
            //遍历每一个未到达基本块
            for (BasicBlock *BB : unreachableSet) //有点像 for i in range(0, 100) : 的画风
            {
                //基本块第一条指令到最后一条指令
                for (i = succ_begin(BB); i != succ_end(BB); i++)
                {
                    i->removePredecessor(BB);
                    // 还会通知该 block 有 predecessor 已经被移除，
                    // 随后这个 block 会检查自己是否有会受到影响的 phi node 并自动做出修改
                }
                BB->eraseFromParent();
            }
            return changed;
        };
    };
} // namespace
​
// LLVM会利用pass的地址来为这个id赋值，所以初始值并不重要
char DeadBlock::ID = 0;

// 注册pass，这个pass可能会改变CFG，所以将第三个参数设为true
static RegisterPass<DeadBlock> X("deadblock", "Dead blocks elimination pass", true, false);
