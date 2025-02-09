/**
 * @file round.cpp
 * @brief find all rounding without specifying round_up or round_down
 *
 */
#include "near_core.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Regex.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

namespace {
    struct Round : public llvm::FunctionPass {
        static char ID;

      private:
        llvm::raw_fd_ostream *os = nullptr;

      public:
        Round() : FunctionPass(ID) {
            std::error_code EC;
            os = new llvm::raw_fd_ostream(std::string(getenv("TMP_DIR")) + std::string("/.round.tmp"), EC, llvm::sys::fs::OpenFlags::OF_Append);
        }
        ~Round() { os->close(); }

        bool runOnFunction(llvm::Function &F) override {
            using namespace llvm;
            if (!Rustle::debug_check_all_func && Rustle::regexForLibFunc.match(F.getName()))
                return false;
            if (Rustle::debug_print_function)
                Rustle::Logger().Debug("Checking function ", F.getName());

            bool found = false;
            for (llvm::BasicBlock &BB : F)
                for (llvm::Instruction &I : BB) {
                    if (!I.getDebugLoc().get() || Rustle::regexForLibLoc.match(I.getDebugLoc().get()->getFilename()))
                        continue;
                    if (Rustle::isInstCallFunc(&I, Rustle::regexRound)) {
                        found = true;
                        Rustle::Logger().Warning("rounding at ", &I.getDebugLoc());
                        *os << F.getName() << "@" << I.getDebugLoc()->getFilename() << "@" << I.getDebugLoc().getLine() << "\n";
                    }
                }
            if (!found && Rustle::debug_print_notfound)
                Rustle::Logger().Info("round not found");

            return false;
        }
    };
}  // namespace

char Round::ID = 0;
static llvm::RegisterPass<Round> X("round", "", false /* Only looks at CFG */, false /* Analysis Pass */);

static llvm::RegisterStandardPasses Y(llvm::PassManagerBuilder::EP_EarlyAsPossible, [](const llvm::PassManagerBuilder &Builder, llvm::legacy::PassManagerBase &PM) { PM.add(new Round()); });
