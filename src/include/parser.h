#pragma once

#include <map>
#include <memory>

namespace AST {
class ExprAST;
class PrototypeAST;
class FunctionAST;
}; // namespace AST

/**handle everything related to parsing the source code*/
namespace Parser {
/**list of precedence of implemented operators*/
extern std::map<char, int> BinopPrecedence;


std::unique_ptr<AST::ExprAST> ParseExpression();
std::unique_ptr<AST::ExprAST> ParseNumberExpr();
std::unique_ptr<AST::ExprAST> ParseParenExpr();
std::unique_ptr<AST::ExprAST> ParseIdentifierExpr();

std::unique_ptr<AST::ExprAST> ParsePrimary();
/**recursivly evaluetes complex expressions into ordered parts*/
std::unique_ptr<AST::ExprAST> ParseBinOpRHS(int ExprPrec,
                                            std::unique_ptr<AST::ExprAST> LHS);
std::unique_ptr<AST::ExprAST> ParseUnary();
std::unique_ptr<AST::ExprAST> ParseIfExpr();
std::unique_ptr<AST::ExprAST> ParseForExpr();

std::unique_ptr<AST::PrototypeAST> ParsePrototype();
std::unique_ptr<AST::FunctionAST> ParseDefinition();
std::unique_ptr<AST::PrototypeAST> ParseExtern();
std::unique_ptr<AST::FunctionAST> ParseTopLevelExpr();

std::unique_ptr<AST::ExprAST> ParseVarExpr();

/**get precedence of current binary operrator*/
int GetTokPrecedence();
}; // namespace Parser
