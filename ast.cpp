#include "ast.h"

#include <vector>
#include <string>

#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"

#include "log.h"
#include "ir.h"

namespace AST {
llvm::Value * NumberExprAST::codegen() {
    return llvm::ConstantFP::get(*IR::Context, llvm::APFloat(Val));
}

llvm::Value * VariableExprAST::codegen() {
    llvm::Value *V = IR::NamedValues[Name];

    if (!V)
        return Log::LogError<llvm::Value*>("Unknonw Variable name");

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
            return Log::LogError<llvm::Value*>("invalid binary operator");
    }
}

llvm::Value * IfExprAST::codegen() {
    llvm::Value * CondV = Cond->codegen();
    if (!CondV) return nullptr;

    CondV = IR::Builder->CreateFCmpONE(CondV, llvm::ConstantFP::get(*IR::Context, llvm::APFloat(0.0)), "ifcond");

    llvm::Function * TheFunction = IR::Builder->GetInsertBlock()->getParent();

    llvm::BasicBlock * ThenBB = llvm::BasicBlock::Create(*IR::Context, "then", TheFunction);
    llvm::BasicBlock * ElseBB = llvm::BasicBlock::Create(*IR::Context, "else");
    llvm::BasicBlock * MergeBB = llvm::BasicBlock::Create(*IR::Context, "ifcont");

    IR::Builder->CreateCondBr(CondV, ThenBB, ElseBB);

    // THEN
    IR::Builder->SetInsertPoint(ThenBB);

    llvm::Value * ThenV = Then->codegen();
    if (!ThenV) return nullptr;

    IR::Builder->CreateBr(MergeBB);
    ThenBB = IR::Builder->GetInsertBlock();


    // ELSE
    TheFunction->insert(TheFunction->end(), ElseBB);
    IR::Builder->SetInsertPoint(ElseBB);

    llvm::Value * ElseV = Else->codegen();
    if (!ElseV) return nullptr;

    IR::Builder->CreateBr(MergeBB);

    ElseBB = IR::Builder->GetInsertBlock();

    // CONTINUE
    TheFunction->insert(TheFunction->end(), MergeBB);
    IR::Builder->SetInsertPoint(MergeBB);
    llvm::PHINode * PN = IR::Builder->CreatePHI(llvm::Type::getDoubleTy(*IR::Context), 2, "iftmp");

    PN->addIncoming(ThenV, ThenBB);
    PN->addIncoming(ElseV, ElseBB);
    return PN;
}

llvm::Value * ForExprAST::codegen() {
    llvm::Value * StartV = Start->codegen();
    if (!StartV) return nullptr;

    llvm::Function * TheFunction = IR::Builder->GetInsertBlock()->getParent();
    llvm::BasicBlock * PreheaderBB = IR::Builder->GetInsertBlock();
    llvm::BasicBlock * LoopBB = llvm::BasicBlock::Create(*IR::Context, "loop", TheFunction);

    IR::Builder->CreateBr(LoopBB);

    IR::Builder->SetInsertPoint(LoopBB);

    llvm::PHINode * Variable = IR::Builder->CreatePHI(llvm::Type::getDoubleTy(*IR::Context), 2, VarName);
    Variable->addIncoming(StartV, PreheaderBB);

    llvm::Value * OldVal = IR::NamedValues[VarName];
    IR::NamedValues[VarName] = Variable;

    if (!Body->codegen()) return nullptr;

    llvm::Value * StepVal = nullptr;

    if (Step) {
        StepVal = Step->codegen();
        if (!StepVal) return nullptr;
    } else {
        StepVal = llvm::ConstantFP::get(*IR::Context, llvm::APFloat(1.0));
    }

    llvm::Value * NextVar = IR::Builder->CreateFAdd(Variable, StepVal, "nextvar");

    llvm::Value * EndCond = End->codegen();
    if (!EndCond)
        return nullptr;

    EndCond = IR::Builder->CreateFCmpONE(EndCond, llvm::ConstantFP::get(*IR::Context, llvm::APFloat(0.0)), "loopcond");

    llvm::BasicBlock * LoopEndBB = IR::Builder->GetInsertBlock();
    llvm::BasicBlock * AfterBB = llvm::BasicBlock::Create(*IR::Context, "afterloop", TheFunction);

    IR::Builder->CreateCondBr(EndCond, LoopBB, AfterBB);

    IR::Builder->SetInsertPoint(AfterBB);

    Variable->addIncoming(NextVar, LoopEndBB);

    if (OldVal)
        IR::NamedValues[VarName] = OldVal;
    else
        IR::NamedValues.erase(VarName);

    return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*IR::Context));
}

llvm::Value * CallExprAST::codegen() {
    llvm::Function * CalleeF = IR::getFunction(Callee);
    if (!CalleeF)
        return Log::LogError<llvm::Value*>("unknown function referenced");

    if (CalleeF->arg_size() != Args.size())
        return Log::LogError<llvm::Value*>("Incorrect number of arguments passed");

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

    auto &P = *Proto;
    IR::FunctionProtos[Proto->getName()] = std::move(Proto);
    llvm::Function * TheFunction = IR::getFunction(P.getName());

    if (!TheFunction) return nullptr;
    if (!TheFunction->empty())
        return (llvm::Function * ) Log::LogError<llvm::Value*>("Function cannot be redefined.");

    llvm::BasicBlock * BB = llvm::BasicBlock::Create(*IR::Context, "entry", TheFunction);
    IR::Builder->SetInsertPoint(BB);

    IR::NamedValues.clear();
    for (auto &Arg : TheFunction->args())
        IR::NamedValues[std::string(Arg.getName())] = &Arg;

    if (llvm::Value * RetVal = Body->codegen()) {
        IR::Builder->CreateRet(RetVal);

        llvm::verifyFunction(*TheFunction);

        IR::FPM->run(*TheFunction);

        return TheFunction;
    }

    TheFunction->eraseFromParent();
    return nullptr;
}
}; //AST
