#pragma once

#include <vector>
#include <memory>
#include <string>

namespace llvm {
    class Value;
    class Function;
};

namespace AST {

class ExprAST {
public:
    virtual ~ExprAST() = default;
    virtual llvm::Value * codegen() = 0;
};

class NumberExprAST : public ExprAST {
    double Val;

public:
    NumberExprAST(double Val) : Val(Val) {}
    llvm::Value * codegen() override;
};

class VariableExprAST : public ExprAST {
    std::string Name;

public:
    VariableExprAST(const std::string & Name) : Name(Name) {};
    llvm::Value * codegen() override;
};

class BinaryExprAST : public ExprAST {
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;

public:
    BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS) : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {};
    llvm::Value * codegen() override;
};

class IfExprAST : public ExprAST {
    std::unique_ptr<ExprAST> Cond, Then, Else;

public:
    IfExprAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<ExprAST> Then, std::unique_ptr<ExprAST> Else) : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {};

    llvm::Value * codegen() override;
};

class ForExprAST : public ExprAST {
    std::string VarName;
    std::unique_ptr<ExprAST> Start, End, Step, Body;

public:
    ForExprAST(const std::string &VarName, std::unique_ptr<ExprAST> Start, std::unique_ptr<ExprAST> End, std::unique_ptr<ExprAST> Step, std::unique_ptr<ExprAST> Body) : VarName(VarName), Start(std::move(Start)), End(std::move(End)), Step(std::move(Step)), Body (std::move(Body)) {};

    llvm::Value * codegen() override;
};

class CallExprAST : public ExprAST {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;

public:
    CallExprAST(const std::string &Callee, std::vector<std::unique_ptr<ExprAST>> Args) : Callee(Callee), Args(std::move(Args)) {}
    llvm::Value * codegen() override;
};

class PrototypeAST {
    std::string Name;
    std::vector<std::string> Args;

public:
    PrototypeAST(const std::string &Name, std::vector<std::string> Args) : Name (Name), Args(std::move(Args)) {}

    const std::string & getName() const {return Name; }

    llvm::Function * codegen();
};

class FunctionAST {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;


public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body) : Proto(std::move(Proto)), Body(std::move(Body)) {}

    llvm::Function * codegen();
};
}; //AST
