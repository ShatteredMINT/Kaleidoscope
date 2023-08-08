#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <utility>

#include <algorithm>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/IR/LegacyPassManager.h"

#include "llvm/Support/Error.h"

#include "include/KaleidoscopeJIT.h"

#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "jit.h"

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/// putchard - putchar that takes a double and returns 0.
extern "C" DLLEXPORT double putchard(double X) {
  fputc((char)X, stderr);
  return 0;
}

/// printd - printf that takes a double prints it as "%f\n", returning 0.
extern "C" DLLEXPORT double printd(double X) {
  fprintf(stderr, "%f\n", X);
  return 0;
}

//===-----------------------------------------
// DRIVER
//===-----------------------------------------

static void HandleDefinition() {
    if (auto FnAST = Parser::ParseDefinition()) {
        if (auto * FnIR = FnAST->codegen()) {
            fprintf(stderr, "Read function definition:\n");
            FnIR->print(llvm::errs());
            fprintf(stderr, "\n");
        }
    } else {
        Lexer::getNextToken();
    }
}

static void HandleExtern() {
    if (auto FnAST = Parser::ParseExtern()) {
        if (auto * FnIR = FnAST->codegen()) {
            fprintf(stderr, "Read extern:\n");
            FnIR->print(llvm::errs());
            fprintf(stderr, "\n");
        }
    } else {
        Lexer::getNextToken();
    }
}

static void HandleTopLevelExpression() {
    if (auto FnAST = Parser::ParseTopLevelExpr()) {
        if (auto * FnIR = FnAST->codegen()) {

            auto RT = JIT::TheJIT->getMainJITDylib().createRessourceTracker();

            auto TSM = llvm::ThreadSafeModule(std::move(IR::Module), std::move(IR::Context));
            JIT::ExitOnErr(JIT::TheJIT->addModule(std::move(TSM), RT));
            IR::InitializeModuleAndPassManager();

            auto ExprSymbol = JIT::ExitOnErr(JIT::TheJIT->lookup("__anon_expr"));
            llvm::assert(ExprSymbol && "Function not found");

            double (*FP) () = ExprSymbol.getAddress().toPtr<double (*) ()>();
            fprintf(stderr, "Evaluated to %f\n", FP());

            JIT::ExitOnErr(RT->remove());

            fprintf(stderr, "Read top-level expression:\n");
            FnIR->print(llvm::errs());
            fprintf(stderr, "\n");

            // FnIR->eraseFromParent();
        }
    } else {
        Lexer::getNextToken();
    }
}

static void MainLoop() {
    while (true) {
        switch (Lexer::CurTok) {
            case tok_eof:
                return;
            case ';':
                Lexer::getNextToken(); // ignore top level smicolons
                break;
            case tok_def:
                HandleDefinition();
                break;
            case tok_extern:
                HandleExtern();
                break;
            default:
                HandleTopLevelExpression();
                break;
        }
        fprintf(stderr, "ready> ");
    }
}

int main() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    Parser::BinopPrecedence['<'] = 10;
    Parser::BinopPrecedence['+'] = 20;
    Parser::BinopPrecedence['-'] = 20;
    Parser::BinopPrecedence['*'] = 40;

    fprintf(stderr, "ready> ");
    Lexer::getNextToken();

    IR::InitializeModuleAndPassManager();

    JIT::TheJIT = JIT::ExitOnErr(llvm::orc::KaleidoscopeJIT::Create());

    MainLoop();

    IR::Module->print(llvm::errs(), nullptr);

    return 0;
}
