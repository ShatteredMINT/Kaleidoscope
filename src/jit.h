#pragma once


#include "include/KaleidoscopeJIT.h"

namespace JIT {
    /** currently used JIT*/
    extern std::unique_ptr<llvm::orc::KaleidoscopeJIT> TheJIT;
    /**simple assertion style error handler*/
    extern llvm::ExitOnError ExitOnErr;

};
