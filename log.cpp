#include "log.h"

#include <memory>

#include "llvm/IR/Constants.h"

#include "ast.h"

std::unique_ptr<AST::ExprAST> Log::LogError(const char *Str) {
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}

std::unique_ptr<AST::PrototypeAST> Log::LogErrorP(const char *Str) {
    LogError(Str);
    return nullptr;
}

llvm::Value * Log::LogErrorV(const char * Str) {
    LogError(Str);
    return nullptr;
}
