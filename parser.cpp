#include "parser.h"

#include <map>
#include <memory>

#include "ast.h"
#include "lexer.h"
#include "log.h"

std::map<char, int> Parser::BinopPrecedence;

std::unique_ptr<AST::ExprAST> Parser::ParseNumberExpr() {
    auto Result = std::make_unique<AST::NumberExprAST>(Lexer::NumVal);
    Lexer::getNextToken();
    return std::move(Result);
}

std::unique_ptr<AST::ExprAST> Parser::ParseParenExpr() {
    Lexer::getNextToken(); //eat '('
    auto V = ParseExpression();
    if (!V)
        return nullptr;

    if (Lexer::CurTok != ')')
        return Log::LogError("expected ')'");
    Lexer::getNextToken();
    return V;
}

std::unique_ptr<AST::ExprAST> Parser::ParseIdentifierExpr() {
    std::string IdName = Lexer::IdentifierStr;
    Lexer::getNextToken(); //eat identifier

    if (Lexer::CurTok != '(') //simple variable ref
        return std::make_unique<AST::VariableExprAST>(IdName);

    std::vector<std::unique_ptr<AST::ExprAST>> Args;

    while (Lexer::CurTok != ')') { // TODO deviate from tutorial
        Lexer::getNextToken();
        if (auto Arg = ParseExpression())
            Args.push_back(std::move(Arg));
        else
            return nullptr;

        if (Lexer::CurTok != ',' && Lexer::CurTok != ')')
            return Log::LogError("Expected ')' or ',' in argument list");
    }

    // Call
    // Lexer::getNextToken(); //eat '('
    // if (Lexer::CurTok != ')') {
    //     while (true) {
    //         if (auto Arg = Parser::ParseExpression())
    //             Args.push_back(std::move(Arg));
    //         else return nullptr;
    //
    //         if (Lexer::CurTok == ')')
    //             break;
    //
    //         if (Lexer::CurTok != ',')
    //             return Log::LogError("test: Expected ')' or ',' in argument list");
    //         Lexer::getNextToken();
    //     }
    // }

    Lexer::getNextToken(); // eat the ')'

    return std::make_unique<AST::CallExprAST>(IdName, std::move(Args));
}


std::unique_ptr<AST::ExprAST> Parser::ParsePrimary() {
    switch (Lexer::CurTok) {
        default:
            return Log::LogError("unknown token when expecting an expression");
        case tok_identifier:
            return ParseIdentifierExpr();
        case tok_number:
            return ParseNumberExpr();
        case '(':
            return ParseParenExpr();
    }
}


int Parser::GetTokPrecedence() {
    if (!isascii(Lexer::CurTok))
        return -1;

    int TokPrec = BinopPrecedence[Lexer::CurTok];
    if (TokPrec <= 0) return -1;
    return TokPrec;
}

std::unique_ptr<AST::ExprAST> Parser::ParseBinOpRHS(int ExprPrec, std::unique_ptr<AST::ExprAST> LHS) {
    while (true) {
        int TokPrec = GetTokPrecedence();

        if (TokPrec < ExprPrec)
            return LHS;

        int BinOp = Lexer::CurTok;
        Lexer::getNextToken(); // eat binop

        auto RHS = ParsePrimary();
        if (!RHS)
            return nullptr;

        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec) {
            RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
            if (!RHS)
                return nullptr;
        }

        LHS = std::make_unique<AST::BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}

std::unique_ptr<AST::ExprAST> Parser::ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS) return nullptr;

    return ParseBinOpRHS(0, std::move(LHS));
}

std::unique_ptr<AST::PrototypeAST> Parser::ParsePrototype() {
    if (Lexer::CurTok != tok_identifier)
        return Log::LogErrorP("Exprected function name in prototype");

    std::string FnName = Lexer::IdentifierStr;
    Lexer::getNextToken();

    if (Lexer::CurTok != '(')
        return Log::LogErrorP("Expected '(' in prototype");

    std::vector<std::string> ArgNames;
    while (Lexer::getNextToken() == tok_identifier)
        ArgNames.push_back(Lexer::IdentifierStr);
    if (Lexer::CurTok != ')')
        return Log::LogErrorP("Expected ')' in prototype");

    Lexer::getNextToken(); // eat ')'

    return std::make_unique<AST::PrototypeAST>(FnName, std::move(ArgNames));
}

std::unique_ptr<AST::FunctionAST> Parser::ParseDefinition() {
    Lexer::getNextToken(); // eat def
    auto Proto = ParsePrototype();
    if (!Proto) return nullptr;

    if (auto E = ParseExpression())
        return std::make_unique<AST::FunctionAST>(std::move(Proto), std::move(E));

    return nullptr;
}

std::unique_ptr<AST::PrototypeAST> Parser::ParseExtern() {
    Lexer::getNextToken(); //eat extern
    return ParsePrototype();
}

std::unique_ptr<AST::FunctionAST> Parser::ParseTopLevelExpr()  {
    if (auto E = ParseExpression()) {
        auto Proto = std::make_unique<AST::PrototypeAST>("__anon_expr", std::vector<std::string>());
        return std::make_unique<AST::FunctionAST>(std::move(Proto), std::move(E));
    }

    return nullptr;
}
