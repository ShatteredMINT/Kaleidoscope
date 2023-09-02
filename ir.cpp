#include "ir.h"

#include <memory>

#include "llvm/IR/LegacyPassManager.h"

#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

#include "jit.h"
#include "ast.h"

std::unique_ptr<llvm::LLVMContext> IR::Context;
std::unique_ptr<llvm::IRBuilder<>> IR::Builder;
std::unique_ptr<llvm::Module> IR::Module;
std::map<std::string, llvm::Value *> IR::NamedValues;
std::map<std::string, std::unique_ptr<PrototypeAST>> IR::FunctionProtos;

std::unique_ptr<llvm::legacy::FunctionPassManager> IR::FPM;

void IR::InitializeModuleAndPassManager() {
    Context = std::make_unique<llvm::LLVMContext>();
    Module = std::make_unique<llvm::Module>("My cool JIT", *IR::Context);

    Module->setDataLayout(JIT::TheJIT->getDataLayout());

    Builder = std::make_unique<llvm::IRBuilder<>>(*IR::Context);

    FPM = std::make_unique<llvm::legacy::FunctionPassManager>(Module.get());

    FPM->add(llvm::createInstructionCombiningPass());
    FPM->add(llvm::createReassociatePass());
    FPM->add(llvm::createGVNPass());
    FPM->add(llvm::createCFGSimplificationPass());

    FPM->doInitialization();
}

llvm::Function* IR::getFunction(std::string Name) {
    if (auto *F = Module->getFunction(Name)) return F;

    auto FI = FunctionProtos.find(Name);
    if (FI != FunctionProtos.end())
        return FI->second->codegen();

    return nullptr;
}
