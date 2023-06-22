#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <map>

class ExprAST;
class FunctionAST;
class PrototypeAST;

class Parser {
public:
    static std::unique_ptr<ExprAST> ParseExpression();
    static std::unique_ptr<ExprAST> ParseNumberExpr();
    static std::unique_ptr<ExprAST> ParseParenExpr();
    static std::unique_ptr<ExprAST> ParseIdentifierExpr();

    static std::unique_ptr<ExprAST> ParsePrimary();
    static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS);

    static std::unique_ptr<PrototypeAST> ParsePrototype();
    static std::unique_ptr<FunctionAST> ParseDefinition();
    static std::unique_ptr<PrototypeAST> ParseExtern();
    static std::unique_ptr<FunctionAST> ParseTopLevelExpr();


    static int GetTokPrecedence();


    static std::map<char, int> BinopPrecedence;
};

#endif
