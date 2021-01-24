// 2.1 Pass
// 1. 初始化
char LoopRotateLegacyPass::ID = 0;

//https://llvm.org/doxygen/PassSupport_8h.html#aaa970fc931c1c63037a8182e028d04b1
//#define 	INITIALIZE_PASS_BEGIN(passName, arg, name, cfg, analysis)   static void *initialize##passName##PassOnce(PassRegistry &Registry) {
INITIALIZE_PASS_BEGIN(LoopRotateLegacyPass, "loop-rotate", "Rotate Loops", false, false)
//#define INITIALIZE_PASS_DEPENDENCY	(	 	depName	)	   initialize##depName##Pass(Registry);
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(LoopPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(MemorySSAWrapperPass)
INITIALIZE_PASS_END(LoopRotateLegacyPass, "loop-rotate", "Rotate Loops", false, false)
Pass *llvm::createLoopRotatePass(int MaxHeaderSize)
{
    return new LoopRotateLegacyPass(MaxHeaderSize);
}

// 2. LoopRotateLegacyPass

class LoopRotateLegacyPass : public LoopPass
{
    unsigned MaxHeaderSize;

public:
    static char ID; // Pass ID, replacement for typeid
    LoopRotateLegacyPass(int SpecifiedMaxHeaderSize = -1) : LoopPass(ID)
    {
        initializeLoopRotateLegacyPassPass(*PassRegistry::getPassRegistry());
        if (SpecifiedMaxHeaderSize == -1) // 默认值
        {
            MaxHeaderSize = DefaultRotationThreshold;
        }
        else
        {
            MaxHeaderSize = unsigned(SpecifiedMaxHeaderSize);
        }
    }

    // what is "LCSSA" ?
    // LCSSA form makes instruction renaming easier.
    void getAnalysisUsage(AnalysisUsage &AU) const override
    {
        AU.addRequired<AssumptionCacheTracker>();
        AU.addRequired<TargetTransformInfoWrapperPass>();
        if (EnableMSSALoopDependency)
        {
            AU.addRequired<MemorySSAWrapperPass>();
            AU.addPreserved<MemorySSAWrapperPass>();
        }
        getLoopAnalysisUsage(AU);
    }

    // 3. runOnLoop
    // 主要的执行函数
    bool runOnLoop(Loop *L, LPPassManager &LPM) override
    {
        if (skipLoop(L))
        {
            return false;
        }
        Function &F = *L->getHeader()->getParent(); //header 的爸爸

        auto *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
        const auto *TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
        auto *AC = &getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F);
        auto *DTWP = getAnalysisIfAvailable<DominatorTreeWrapperPass>();
        auto *DT = DTWP ? &DTWP->getDomTree() : nullptr; // 三元式换掉
        auto *SEWP = getAnalysisIfAvailable<ScalarEvolutionWrapperPass>();
        auto *SE = SEWP ? &SEWP->getSE() : nullptr; // 三元式换掉
        const SimplifyQuery SQ = getBestSimplifyQuery(*this, F);
        Optional<MemorySSAUpdater> MSSAU;
        if (EnableMSSALoopDependency)
        {
            MemorySSA *MSSA = &getAnalysis<MemorySSAWrapperPass>().getMSSA();
            MSSAU = MemorySSAUpdater(MSSA);
        }
        return LoopRotation(L, LI, TTI, AC, DT, SE,
                            MSSAU.hasValue() ? MSSAU.getPointer() : nullptr, SQ,
                            false, MaxHeaderSize, false);
    }
};

//2.2 Loops Rotate 动作相关代码
//    1.LoopRotation 类：
class LoopRotate
{
    const unsigned MaxHeaderSize;
    LoopInfo *LI;
    const TargetTransformInfo *TTI;
    AssumptionCache *AC;
    DominatorTree *DT;
    ScalarEvolution *SE;
    MemorySSAUpdater *MSSAU;
    const SimplifyQuery &SQ;
    bool RotationOnly;
    bool IsUtilMode;

public:
    LoopRotate(unsigned MaxHeaderSize, LoopInfo *LI,
               const TargetTransformInfo *TTI, AssumptionCache *AC,
               DominatorTree *DT, ScalarEvolution *SE, MemorySSAUpdater *MSSAU,
               const SimplifyQuery &SQ, bool RotationOnly, bool IsUtilMode)
        : MaxHeaderSize(MaxHeaderSize), LI(LI), TTI(TTI), AC(AC), DT(DT), SE(SE),
          MSSAU(MSSAU), SQ(SQ), RotationOnly(RotationOnly),
          IsUtilMode(IsUtilMode) {}
    bool processLoop(Loop *L);

private:
    bool rotateLoop(Loop *L, bool SimplifiedLatch);
    bool simplifyLoopLatch(Loop *L);
};

//      2. processLoop：这玩意真的在外面么？
bool LoopRotate::processLoop(Loop *L)
{
    // Save the loop metadata.
    MDNode *LoopMD = L->getLoopID();

    bool SimplifiedLatch = false;

    // Simplify the loop latch before attempting to rotate the header
    // upward. Rotation may not be needed if the loop tail can be folded into the
    // loop exit.
    if (!RotationOnly)
    {
        SimplifiedLatch = simplifyLoopLatch(L);
    }
    
    bool MadeChange = rotateLoop(L, SimplifiedLatch);
    assert((!MadeChange || L->isLoopExiting(L->getLoopLatch())) &&
           "Loop latch should be exiting after loop-rotate.");

    // Restore the loop metadata.
    // NB! We presume LoopRotation DOESN'T ADD its own metadata.
    if ((MadeChange || SimplifiedLatch) && LoopMD)
    {
        L->setLoopID(LoopMD);   
    }
    return MadeChange || SimplifiedLatch;
}
