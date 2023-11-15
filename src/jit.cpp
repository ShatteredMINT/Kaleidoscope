#include "include/jit.h"

// "initizalization"
std::unique_ptr<llvm::orc::KaleidoscopeJIT> JIT::TheJIT;
llvm::ExitOnError JIT::ExitOnErr;
