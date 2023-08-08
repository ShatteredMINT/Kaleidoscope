#pragma once


#include "include/KaleidoscopeJIT.h"

struct JIT {
    static std::unique_ptr<llvm::orc::KaleidoscopeJIT> TheJIT;
    static llvm::ExitOnError ExitOnErr;

};
