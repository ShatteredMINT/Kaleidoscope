#pragma once


#include "include/KaleidoscopeJIT.h"

namespace JIT {
    extern std::unique_ptr<llvm::orc::KaleidoscopeJIT> TheJIT;
    extern llvm::ExitOnError ExitOnErr;

};
