#include "include/ast.h"

#include <vector>
#include <string>

#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"

#include "include/log.h"
#include "include/ir.h"
#include "include/parser.h"

namespace AST {
llvm::Value * NumberExprAST::codegen() {
    return llvm::ConstantFP::get(*IR::Context, llvm::APFloat(Val));
}

llvm::Value * VariableExprAST::codegen() {
    llvm::Value *V = IR::NamedValues[Name];

    if (!V)
        return Log::LogError<llvm::Value*>("Unknown Variable name");

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
            break;
    }

    llvm::Function * F = IR::getFunction(std::string("binary") + Op);
    assert(F && "binary operator not found!");

    llvm::Value * Ops[2] = { L, R };
    return IR::Builder->CreateCall(F, Ops, "binop");
}

llvm::Value * UnaryExprAST::codegen() {
    llvm::Value * OperandV = Operand->codegen();
    if (!OperandV)
        return nullptr;

    llvm::Function * F = IR::getFunction(std::string("unary") + Opcode);
    if (!F)
        return Log::LogError<llvm::Value*>("Unknown unary Operator");

    return IR::Builder->CreateCall(F, OperandV, "unop");
}

llvm::Value * IfExprAST::codegen() {
    llvm::Value * CondV = Cond->codegen();
    if (!CondV) return nullptr;

    CondV = IR::Builder->CreateFCmpONE(CondV, llvm::ConstantFP::get(*IR::Context, llvm::APFloat(0.0)), "ifcond");

    // set up code block structure
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

    // setup code blocks
    llvm::Function * TheFunction = IR::Builder->GetInsertBlock()->getParent();
    llvm::BasicBlock * PreheaderBB = IR::Builder->GetInsertBlock();
    llvm::BasicBlock * LoopBB = llvm::BasicBlock::Create(*IR::Context, "loop", TheFunction);

    IR::Builder->CreateBr(LoopBB);

    // LOOP
    IR::Builder->SetInsertPoint(LoopBB);

    llvm::PHINode * Variable = IR::Builder->CreatePHI(llvm::Type::getDoubleTy(*IR::Context), 2, VarName);
    Variable->addIncoming(StartV, PreheaderBB);

    llvm::Value * OldVal = IR::NamedValues[VarName];
    IR::NamedValues[VarName] = Variable;

    if (!Body->codegen()) return nullptr;

    llvm::Value * StepVal = nullptr;

    // support optional step size
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


    // additional code blocks to support branch after iteration
    llvm::BasicBlock * LoopEndBB = IR::Builder->GetInsertBlock();
    llvm::BasicBlock * AfterBB = llvm::BasicBlock::Create(*IR::Context, "afterloop", TheFunction);

    IR::Builder->CreateCondBr(EndCond, LoopBB, AfterBB);

    // AFTER
    IR::Builder->SetInsertPoint(AfterBB);

    Variable->addIncoming(NextVar, LoopEndBB);

    if (OldVal)
        IR::NamedValues[VarName] = OldVal;
    else
        IR::NamedValues.erase(VarName);

    return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*IR::Context));
}

llvm::Value * CallExprAST::codegen() {
    // check if function exists
    llvm::Function * CalleeF = IR::getFunction(Callee);
    if (!CalleeF)
        return Log::LogError<llvm::Value*>("unknown function referenced");

    // validate call
    if (CalleeF->arg_size() != Args.size())
        return Log::LogError<llvm::Value*>("Incorrect number of arguments passed");

    // construct call
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

    // check if function has prototype
    if (!TheFunction) return nullptr;
    if (!TheFunction->empty())
        return (llvm::Function * ) Log::LogError<llvm::Value*>("Function cannot be redefined.");

    if (P.isBinaryOp())
        Parser::BinopPrecedence[P.getOperatorName()] = P.getBinaryPrecedence();

    // create root block for junction
    llvm::BasicBlock * BB = llvm::BasicBlock::Create(*IR::Context, "entry", TheFunction);
    IR::Builder->SetInsertPoint(BB);

    // make local variables and add args to it
    IR::NamedValues.clear();
    for (auto &Arg : TheFunction->args())
        IR::NamedValues[std::string(Arg.getName())] = &Arg;

    // create function code
    if (llvm::Value * RetVal = Body->codegen()) {
        IR::Builder->CreateRet(RetVal);

        llvm::verifyFunction(*TheFunction);

        IR::FPM->run(*TheFunction);

        return TheFunction;
    }

    // if code generation fails cleanup
    TheFunction->eraseFromParent();
    return nullptr;
}
}; //AST
