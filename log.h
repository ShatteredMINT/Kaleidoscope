#pragma once

#include <memory>

namespace llvm {class Value;};

namespace AST {
    class ExprAST;
    class PrototypeAST;
};
namespace Log {
    std::unique_ptr<AST::ExprAST> LogError(const char *Str);
    std::unique_ptr<AST::PrototypeAST> LogErrorP(const char *Str);
    llvm::Value * LogErrorV(const char * Str);
};
