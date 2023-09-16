#pragma once

#include <vector>
#include <memory>
#include <string>
#include <cassert>

namespace llvm {
    class Value;
    class Function;
};

/**
 * contains AST node classes
 */
namespace AST {

class ExprAST {
public:
    virtual ~ExprAST() = default;
    virtual llvm::Value * codegen() = 0;
};

/**
 * contains single numeric value
 */
class NumberExprAST : public ExprAST {
    double Val;

public:
    NumberExprAST(double Val) : Val(Val) {}
    llvm::Value * codegen() override;
};

/**
 * contains reference to a Variable
 */
class VariableExprAST : public ExprAST {
    std::string Name;

public:
    VariableExprAST(const std::string & Name) : Name(Name) {};
    llvm::Value * codegen() override;
};

/**
 * contains Binary operations (can hold other expressions!)
 */
class BinaryExprAST : public ExprAST {
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;

public:
    BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS) : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {};
    llvm::Value * codegen() override;
};

/**
 * contains if else statement (can hold other expressions!)
 */
class IfExprAST : public ExprAST {
    std::unique_ptr<ExprAST> Cond, Then, Else;

public:
    IfExprAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<ExprAST> Then, std::unique_ptr<ExprAST> Else) : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {};

    llvm::Value * codegen() override;
};

/**
 * contains for loop (holds other expressions!)
 */
class ForExprAST : public ExprAST {
    std::string VarName;
    std::unique_ptr<ExprAST> Start, End, Step, Body;

public:
    ForExprAST(const std::string &VarName, std::unique_ptr<ExprAST> Start, std::unique_ptr<ExprAST> End, std::unique_ptr<ExprAST> Step, std::unique_ptr<ExprAST> Body) : VarName(VarName), Start(std::move(Start)), End(std::move(End)), Step(std::move(Step)), Body (std::move(Body)) {};

    llvm::Value * codegen() override;
};

/**
 * contains function Callee
 */
class CallExprAST : public ExprAST {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;

public:
    CallExprAST(const std::string &Callee, std::vector<std::unique_ptr<ExprAST>> Args) : Callee(Callee), Args(std::move(Args)) {}
    llvm::Value * codegen() override;
};

/**
 * contains function head
 */
class PrototypeAST {
    std::string Name;
    std::vector<std::string> Args;

    bool IsOperator;
    unsigned Precedence;

public:
    PrototypeAST(const std::string &Name, std::vector<std::string> Args, bool IsOperator = false, unsigned Prec = 0) : Name (Name), Args(std::move(Args)), IsOperator(IsOperator), Precedence(Prec) {}

    const std::string & getName() const {return Name; }
    llvm::Function * codegen();

    bool isUnaryOp() const {
        return IsOperator && Args.size() == 1;
    }
    bool isBinaryOp() const {
        return IsOperator && Args.size() == 2;
    }

    char getOperatorName() const {
        assert(isUnaryOp() || isBinaryOp());
        return Name[Name.size() - 1];
    }

    unsigned getBinaryPrecedence() const {return Precedence;}
};

/**
 * combines function body and head
 */
class FunctionAST {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;


public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body) : Proto(std::move(Proto)), Body(std::move(Body)) {}

    llvm::Function * codegen();
};
}; //AST
