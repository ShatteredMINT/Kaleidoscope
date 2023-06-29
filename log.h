#ifndef LOG_H
#define LOG_H


#include <memory>

#include "llvm/IR/Constants.h"

class ExprAST;
class PrototypeAST;

namespace Log {
    std::unique_ptr<ExprAST> LogError(const char *Str);
    std::unique_ptr<PrototypeAST> LogErrorP(const char *Str);
    llvm::Value * LogErrorV(const char * Str);
};

#endif
