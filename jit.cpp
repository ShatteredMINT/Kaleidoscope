#include "jit.h"


std::unique_ptr<llvm::orc::KaleidoscopeJIT> JIT::TheJIT;
llvm::ExitOnError JIT::ExitOnErr;
