#ifndef LOG_H
#define LOG_H


#include <memory>

class ExprAST;
class PrototypeAST;

class Log {
public:
    static std::unique_ptr<ExprAST> LogError(const char *Str);
    static std::unique_ptr<PrototypeAST> LogErrorP(const char *Str);
};

#endif
