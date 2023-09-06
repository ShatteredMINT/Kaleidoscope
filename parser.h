#pragma once

#include <memory>
#include <map>

namespace AST {
    class ExprAST;
    class PrototypeAST;
    class FunctionAST;
};

namespace Parser {
     std::unique_ptr<AST::ExprAST> ParseExpression();
     std::unique_ptr<AST::ExprAST> ParseNumberExpr();
     std::unique_ptr<AST::ExprAST> ParseParenExpr();
     std::unique_ptr<AST::ExprAST> ParseIdentifierExpr();

     std::unique_ptr<AST::ExprAST> ParsePrimary();
     std::unique_ptr<AST::ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<AST::ExprAST> LHS);
     std::unique_ptr<AST::ExprAST> ParseIfExpr();
     std::unique_ptr<AST::ExprAST> ParseForExpr();

     std::unique_ptr<AST::PrototypeAST> ParsePrototype();
     std::unique_ptr<AST::FunctionAST> ParseDefinition();
     std::unique_ptr<AST::PrototypeAST> ParseExtern();
     std::unique_ptr<AST::FunctionAST> ParseTopLevelExpr();


     int GetTokPrecedence();


     extern std::map<char, int> BinopPrecedence;
};
