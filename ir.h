#pragma once

#include<map>
#include<memory>

#include "llvm/IR/IRBuilder.h"

// forward declarations
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

/**
 * contains everything related to IR generation and its usage
 */
namespace IR{
    extern std::unique_ptr<llvm::LLVMContext> Context;
    extern std::unique_ptr<llvm::IRBuilder<>> Builder;
    extern std::unique_ptr<llvm::Module> Module;

    // TODO should probably be moved into namespace AST
    /**list of existing variables*/
    extern std::map<std::string, llvm::Value *> NamedValues;
    // TODO should probably be moved into namespace AST
    /** list of already declared functions*/
    extern std::map<std::string, std::unique_ptr<AST::PrototypeAST>> FunctionProtos;

    /**optimization passes*/
    extern std::unique_ptr<llvm::legacy::FunctionPassManager> FPM;

    /**setup llvm toolchain to be ready for code creation and execution*/
    void InitializeModuleAndPassManager();

    // TODO should probably be moved into namespace AST
    /**returns the most recent funtion fitting the name*/
    llvm::Function * getFunction(std::string Name);
};
