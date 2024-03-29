#include "include/ir.h"

#include <memory>

#include "llvm/IR/LegacyPassManager.h"

#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

#include "include/jit.h"
#include "include/ast.h"

// "initialize" values
std::unique_ptr<llvm::LLVMContext> IR::Context;
std::unique_ptr<llvm::IRBuilder<>> IR::Builder;
std::unique_ptr<llvm::Module> IR::Module;
std::map<std::string, llvm::AllocaInst *> IR::NamedValues;
std::map<std::string, std::unique_ptr<AST::PrototypeAST>> IR::FunctionProtos;

std::unique_ptr<llvm::legacy::FunctionPassManager> IR::FPM;

void IR::InitializeModuleAndPassManager() {
    // setup toolchain
    Context = std::make_unique<llvm::LLVMContext>();
    Module = std::make_unique<llvm::Module>("My cool JIT", *IR::Context);

    Module->setDataLayout(JIT::TheJIT->getDataLayout());

    Builder = std::make_unique<llvm::IRBuilder<>>(*IR::Context);

    //add simple optimizations
    FPM = std::make_unique<llvm::legacy::FunctionPassManager>(Module.get());

    FPM->add(llvm::createInstructionCombiningPass());
    FPM->add(llvm::createReassociatePass());
    FPM->add(llvm::createGVNPass());
    FPM->add(llvm::createCFGSimplificationPass());

    FPM->doInitialization();
} 

llvm::AllocaInst * IR::CreateEntryBlockAlloca(llvm::Function *TheFunction, llvm::StringRef VarName) {
  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(llvm::Type::getDoubleTy(* Context), nullptr, VarName);
}

llvm::Function* IR::getFunction(std::string Name) {
    if (auto *F = Module->getFunction(Name)) return F;

    auto FI = FunctionProtos.find(Name);
    if (FI != FunctionProtos.end())
        return FI->second->codegen();

    return nullptr;
}
