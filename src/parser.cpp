#include "include/parser.h"

#include <map>
#include <memory>

#include "include/ast.h"
#include "include/lexer.h"
#include "include/log.h"

std::map<char, int> Parser::BinopPrecedence = {
    {'<', 10},
    {'+', 20},
    {'-', 20},
    {'*', 40}
};

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
        return Log::LogError<std::unique_ptr<AST::ExprAST>>("expected ')'");
    Lexer::getNextToken();
    return V;
}

std::unique_ptr<AST::ExprAST> Parser::ParseIdentifierExpr() {
    std::string IdName = Lexer::IdentifierStr;
    Lexer::getNextToken(); //eat identifier

    // simple variable use
    if (Lexer::CurTok != '(')
        return std::make_unique<AST::VariableExprAST>(IdName);

    // function call
    // parse Args
    std::vector<std::unique_ptr<AST::ExprAST>> Args;

    // REMINDER deviate from tutorial
    while (Lexer::CurTok != ')') {
        Lexer::getNextToken();
        if (auto Arg = ParseExpression())
            Args.push_back(std::move(Arg));
        else
            return nullptr;

        if (Lexer::CurTok != ',' && Lexer::CurTok != ')')
            return Log::LogError<std::unique_ptr<AST::ExprAST>>("Expected ')' or ',' in argument list");
    }

    // Reference implementation
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
            return Log::LogError<std::unique_ptr<AST::ExprAST>>("unknown token when expecting an expression");
        case tok_identifier:
            return ParseIdentifierExpr();
        case tok_number:
            return ParseNumberExpr();
        case '(':
            return ParseParenExpr();
        case tok_if:
            return ParseIfExpr();
        case tok_for:
            return ParseForExpr();
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

        // existing value needs to be parsed first
        if (TokPrec < ExprPrec)
            return LHS;

        // parse next expression
        int BinOp = Lexer::CurTok;
        Lexer::getNextToken(); // eat binop

        auto RHS = ParseUnary();
        if (!RHS)
            return nullptr;

        // next operation need to be parsed first
        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec) {
            RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
            if (!RHS)
                return nullptr;
        }

        // add node to tree
        LHS = std::make_unique<AST::BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}

std::unique_ptr<AST::ExprAST> Parser::ParseUnary() {
    //check if it is not an operator
    if (!isascii(Lexer::CurTok) || Lexer::CurTok == '(' || Lexer::CurTok == '.')
        return ParsePrimary();

    // proceed with unary parsing

    char Opc = Lexer::CurTok;
    Lexer::getNextToken();
    if (auto Operand = ParseUnary())
        return std::make_unique<AST::UnaryExprAST>(Opc, std::move(Operand));
    return nullptr;
}

std::unique_ptr<AST::ExprAST> Parser::ParseIfExpr() {
    Lexer::getNextToken(); //eat IF

    // COND
    auto Cond = ParseExpression();
    if (!Cond) return nullptr;

    // THEN
    if (Lexer::CurTok != tok_then)
        return Log::LogError<std::unique_ptr<AST::ExprAST>>("Expected then!");
    Lexer::getNextToken(); //eat THEN

    auto Then = ParseExpression();
    if (!Then) return nullptr;

    // ELSE
    if (Lexer::CurTok != tok_else)
        return Log::LogError<std::unique_ptr<AST::ExprAST>>("Expected else!");

    Lexer::getNextToken();

    auto Else = ParseExpression();
    if (!Else) return nullptr;

    return std::make_unique<AST::IfExprAST>(std::move(Cond), std::move(Then), std::move(Else));
}

std::unique_ptr<AST::ExprAST> Parser::ParseForExpr() {
    Lexer::getNextToken(); //eat FOR

    // setup loop variable
    if (Lexer::CurTok != tok_identifier)
        return Log::LogError<std::unique_ptr<AST::ExprAST>>("expected identifier after for");

    std::string IdName = Lexer::IdentifierStr;
    Lexer::getNextToken(); //eat identifier

    if (Lexer::CurTok != '=')
        return Log::LogError<std::unique_ptr<AST::ExprAST>>("expected '=' after for");
    Lexer::getNextToken(); //eat =

    auto Start = ParseExpression();
    if (!Start) return nullptr;

    // setup condition
    if (Lexer::CurTok != ',')
        return Log::LogError<std::unique_ptr<AST::ExprAST>>("expected ',' after for start value");
    Lexer::getNextToken(); //eat ','

    auto End = ParseExpression();
    if (!End) return nullptr;

    // STEP is optional
    std::unique_ptr<AST::ExprAST> Step;
    if (Lexer::CurTok == ',') {
        Lexer::getNextToken();
        Step = ParseExpression();
        if (!Step) return nullptr;
    }

    // BODY
    if (Lexer::CurTok != tok_in)
        return Log::LogError<std::unique_ptr<AST::ExprAST>>("expected 'in' after for");
    Lexer::getNextToken(); // eat 'in'

    auto Body = ParseExpression();
    if (!Body) return nullptr;

    return std::make_unique<AST::ForExprAST>(IdName, std::move(Start), std::move(End), std::move(Step), std::move(Body));
}

std::unique_ptr<AST::ExprAST> Parser::ParseExpression() {
    auto LHS = ParseUnary();
    if (!LHS) return nullptr;

    return ParseBinOpRHS(0, std::move(LHS));
}

std::unique_ptr<AST::PrototypeAST> Parser::ParsePrototype() {
    std::string FnName;

    unsigned Kind = 0;
    unsigned BinaryPrecedence = 30;

    switch (Lexer::CurTok) {
        default:
            return Log::LogError<std::unique_ptr<AST::PrototypeAST>>("Expected function name in prototype");

            // normal Function
        case tok_identifier:
            FnName = Lexer::IdentifierStr;
            Kind = 0;
            Lexer::getNextToken(); // eat IdentifierStr
            break;

        case tok_unary:
            Lexer::getNextToken();
            if (!isascii(Lexer::CurTok))
                return Log::LogError<std::unique_ptr<AST::PrototypeAST>>("Expected unary operator!");

            FnName = "unary";
            FnName += (char)Lexer::CurTok;
            Kind = 1;
            Lexer::getNextToken();
            break;

            // binary operator
        case tok_binary:
            Lexer::getNextToken(); // eat 'binary'
            if (!isascii(Lexer::CurTok))
                return Log::LogError<std::unique_ptr<AST::PrototypeAST>>("Expected binary operator");
            FnName = "binary";
            FnName += (char)Lexer::CurTok;
            Kind = 2;
            Lexer::getNextToken(); // eat operator

            if (Lexer::CurTok == tok_number) {
                if (Lexer::NumVal < 1 || Lexer::NumVal > 100)
                    return Log::LogError<std::unique_ptr<AST::PrototypeAST>>("Invalid precedence: must be 1..100");
                BinaryPrecedence = (unsigned)Lexer::NumVal;
                Lexer::getNextToken(); // eat precedence
            }
            break;
    }

    if (Lexer::CurTok != '(')
        return Log::LogError<std::unique_ptr<AST::PrototypeAST>>("Expected '(' in prototype");

    //TODO extend to expect ',' seperation
    std::vector<std::string> ArgNames;
    while (Lexer::getNextToken() == tok_identifier)
        ArgNames.push_back(Lexer::IdentifierStr);
    if (Lexer::CurTok != ')')
        return Log::LogError<std::unique_ptr<AST::PrototypeAST>>("Expected ')' in prototype");

    Lexer::getNextToken(); // eat ')'

    if (Kind && ArgNames.size() != Kind)
        return Log::LogError<std::unique_ptr<AST::PrototypeAST>>("Invalid number of operands for operator");

    return std::make_unique<AST::PrototypeAST>(FnName, std::move(ArgNames), Kind != 0, BinaryPrecedence);
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
        // translate expression into anonymous function
        auto Proto = std::make_unique<AST::PrototypeAST>("__anon_expr", std::vector<std::string>());
        return std::make_unique<AST::FunctionAST>(std::move(Proto), std::move(E));
    }

    return nullptr;
}
