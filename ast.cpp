#include "ast.h"

#include <string>

#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include "log.h"

std::unique_ptr<llvm::LLVMContext> AST::Context;
std::unique_ptr<llvm::IRBuilder<>> AST::Builder;
std::unique_ptr<llvm::Module> AST::Module;
std::map<std::string, llvm::Value *> AST::NamedValues;

llvm::Value * NumberExprAST::codegen() {
    return llvm::ConstantFP::get(*AST::Context, llvm::APFloat(Val));
}

llvm::Value * VariableExprAST::codegen() {
    llvm::Value *V = AST::NamedValues[Name];

    if (!V)
        return Log::LogErrorV("Unknonw Variable name");

    return V;
}

llvm::Value * BinaryExprAST::codegen() {
    llvm::Value * L = LHS->codegen();
    llvm::Value * R = RHS->codegen();
    if (!L || !R) return nullptr;

    switch (Op) {
        case '+':
            return AST::Builder->CreateFAdd(L, R, "addtmp");
        case '-':
            return AST::Builder->CreateFSub(L, R, "subtmp");
        case '*':
            return AST::Builder->CreateFMul(L, R, "multmp");
        case '<':
            L = AST::Builder->CreateFCmpULT(L, R, "cmptmp");
            return AST::Builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*AST::Context), "booltmp");

        default:
            return Log::LogErrorV("invalid binary operator");
    }
}

llvm::Value * CallExprAST::codegen() {
    llvm::Function * CalleeF = AST::Module->getFunction(Callee);
    if (!CalleeF)
        return Log::LogErrorV("unknown function referenced");

    if (CalleeF->arg_size() != Args.size())
        return Log::LogErrorV("Incorrect number of arguments passed");

    std::vector<llvm::Value *> ArgsV;
    for (unsigned i = 0, e = Args.size(); i != e; ++i) {
        ArgsV.push_back(Args[i]->codegen());
        if (!ArgsV.back()) return nullptr;
    }

    return AST::Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

llvm::Function * PrototypeAST::codegen() {
    std::vector<llvm::Type *> Doubles(Args.size(), llvm::Type::getDoubleTy(*AST::Context));

    llvm::FunctionType * FT = llvm::FunctionType::get(llvm::Type::getDoubleTy(*AST::Context), Doubles, false);

    llvm::Function * F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, AST::Module.get());

    unsigned Idx = 0;
    for (auto & Arg : F->args())
        Arg.setName(Args[Idx++]);

    return F;
}

llvm::Function * FunctionAST::codegen() {

    llvm::Function * TheFunction = AST::Module->getFunction(Proto->getName());

    if (!TheFunction)
        TheFunction = Proto->codegen();
    if (!TheFunction) return nullptr;
    if (!TheFunction->empty())
        return (llvm::Function * ) Log::LogErrorV("Function cannot be redefined.");

    llvm::BasicBlock * BB = llvm::BasicBlock::Create(*AST::Context, "entry", TheFunction);
    AST::Builder->SetInsertPoint(BB);

    AST::NamedValues.clear();
    for (auto &Arg : TheFunction->args())
        AST::NamedValues[std::string(Arg.getName())] = &Arg;

    if (llvm::Value * RetVal = Body->codegen()) {
        AST::Builder->CreateRet(RetVal);

        llvm::verifyFunction(*TheFunction);

        return TheFunction;
    }

    TheFunction->eraseFromParent();
    return nullptr;
}
