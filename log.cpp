#include "log.h"

#include <memory>

#include "ast.h"

std::unique_ptr<ExprAST> Log::LogError(const char *Str) {
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}

std::unique_ptr<PrototypeAST> Log::LogErrorP(const char *Str) {
    LogError(Str);
    return nullptr;
}
