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

namespace AST{
    class PrototypeAST;
};

namespace IR{
    extern std::unique_ptr<llvm::LLVMContext> Context;
    extern std::unique_ptr<llvm::IRBuilder<>> Builder;
    extern std::unique_ptr<llvm::Module> Module;
    extern std::map<std::string, llvm::Value *> NamedValues;
    extern std::map<std::string, std::unique_ptr<AST::PrototypeAST>> FunctionProtos;

    extern std::unique_ptr<llvm::legacy::FunctionPassManager> FPM;

    void InitializeModuleAndPassManager();

    llvm::Function * getFunction(std::string Name);
};
