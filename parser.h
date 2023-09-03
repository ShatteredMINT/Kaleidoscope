#pragma once

#include <memory>
#include <map>

namespace AST {
    class ExprAST;
    class PrototypeAST;
    class FunctionAST;
};

class Parser {
public:
    static std::unique_ptr<AST::ExprAST> ParseExpression();
    static std::unique_ptr<AST::ExprAST> ParseNumberExpr();
    static std::unique_ptr<AST::ExprAST> ParseParenExpr();
    static std::unique_ptr<AST::ExprAST> ParseIdentifierExpr();

    static std::unique_ptr<AST::ExprAST> ParsePrimary();
    static std::unique_ptr<AST::ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<AST::ExprAST> LHS);

    static std::unique_ptr<AST::PrototypeAST> ParsePrototype();
    static std::unique_ptr<AST::FunctionAST> ParseDefinition();
    static std::unique_ptr<AST::PrototypeAST> ParseExtern();
    static std::unique_ptr<AST::FunctionAST> ParseTopLevelExpr();


    static int GetTokPrecedence();


    static std::map<char, int> BinopPrecedence;
};
