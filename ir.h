#pragma once

#include<map>
#include<memory>

#include "llvm/IR/IRBuilder.h"

namespace llvm {
    namespace legacy {
        class FunctionPassManager;
    };

    class LLVMContext;
    class Module;
    class Value;
    class Function;
};

class PrototypeAST;

class IR{
public:
    static std::unique_ptr<llvm::LLVMContext> Context;
    static std::unique_ptr<llvm::IRBuilder<>> Builder;
    static std::unique_ptr<llvm::Module> Module;
    static std::map<std::string, llvm::Value *> NamedValues;
    static std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;

    static std::unique_ptr<llvm::legacy::FunctionPassManager> FPM;

    static void InitializeModuleAndPassManager();

    static llvm::Function * getFunction(std::string Name);
};
