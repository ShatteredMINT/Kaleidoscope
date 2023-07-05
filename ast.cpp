#include "ast.h"

#include <string>

#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include "log.h"

std::unique_ptr<llvm::LLVMContext> IR::Context;
std::unique_ptr<llvm::IRBuilder<>> IR::Builder;
std::unique_ptr<llvm::Module> IR::Module;
std::map<std::string, llvm::Value *> IR::NamedValues;

void IR::InitializeModuleAndPassManager() {
    Context = std::make_unique<llvm::LLVMContext>();
    Module = std::make_unique<llvm::Module>("My cool JIT", *IR::Context);
    Builder = std::make_unique<llvm::IRBuilder<>>(*IR::Context);
}


llvm::Value * NumberExprAST::codegen() {
    return llvm::ConstantFP::get(*IR::Context, llvm::APFloat(Val));
}

llvm::Value * VariableExprAST::codegen() {
    llvm::Value *V = IR::NamedValues[Name];

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
            return IR::Builder->CreateFAdd(L, R, "addtmp");
        case '-':
            return IR::Builder->CreateFSub(L, R, "subtmp");
        case '*':
            return IR::Builder->CreateFMul(L, R, "multmp");
        case '<':
            L = IR::Builder->CreateFCmpULT(L, R, "cmptmp");
            return IR::Builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*IR::Context), "booltmp");

        default:
            return Log::LogErrorV("invalid binary operator");
    }
}

llvm::Value * CallExprAST::codegen() {
    llvm::Function * CalleeF = IR::Module->getFunction(Callee);
    if (!CalleeF)
        return Log::LogErrorV("unknown function referenced");

    if (CalleeF->arg_size() != Args.size())
        return Log::LogErrorV("Incorrect number of arguments passed");

    std::vector<llvm::Value *> ArgsV;
    for (unsigned i = 0, e = Args.size(); i != e; ++i) {
        ArgsV.push_back(Args[i]->codegen());
        if (!ArgsV.back()) return nullptr;
    }

    return IR::Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

llvm::Function * PrototypeAST::codegen() {
    std::vector<llvm::Type *> Doubles(Args.size(), llvm::Type::getDoubleTy(*IR::Context));

    llvm::FunctionType * FT = llvm::FunctionType::get(llvm::Type::getDoubleTy(*IR::Context), Doubles, false);

    llvm::Function * F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, IR::Module.get());

    unsigned Idx = 0;
    for (auto & Arg : F->args())
        Arg.setName(Args[Idx++]);

    return F;
}

llvm::Function * FunctionAST::codegen() {

    llvm::Function * TheFunction = IR::Module->getFunction(Proto->getName());

    if (!TheFunction)
        TheFunction = Proto->codegen();
    if (!TheFunction) return nullptr;
    if (!TheFunction->empty())
        return (llvm::Function * ) Log::LogErrorV("Function cannot be redefined.");

    llvm::BasicBlock * BB = llvm::BasicBlock::Create(*IR::Context, "entry", TheFunction);
    IR::Builder->SetInsertPoint(BB);

    IR::NamedValues.clear();
    for (auto &Arg : TheFunction->args())
        IR::NamedValues[std::string(Arg.getName())] = &Arg;

    if (llvm::Value * RetVal = Body->codegen()) {
        IR::Builder->CreateRet(RetVal);

        llvm::verifyFunction(*TheFunction);

        return TheFunction;
    }

    TheFunction->eraseFromParent();
    return nullptr;
}
