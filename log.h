#pragma once

#include <memory>

namespace llvm {class Value;};

class ExprAST;
class PrototypeAST;

namespace Log {
    std::unique_ptr<ExprAST> LogError(const char *Str);
    std::unique_ptr<PrototypeAST> LogErrorP(const char *Str);
    llvm::Value * LogErrorV(const char * Str);
};
