#include "llvm/Support/TargetSelect.h"

#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "jit.h"
#include "ir.h"

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

            //output llvm IR
            fprintf(stderr, "Read function definition:\n");
            FnIR->print(llvm::errs());
            fprintf(stderr, "\n");

            // put function into seperate module and create new module for future code
            JIT::ExitOnErr(JIT::TheJIT->addModule(llvm::orc::ThreadSafeModule(std::move(IR::Module), std::move(IR::Context))));
            IR::InitializeModuleAndPassManager();
        }
    } else {
        Lexer::getNextToken();
    }
}

static void HandleExtern() {
    if (auto ProtoAST = Parser::ParseExtern()) {
        if (auto * FnIR = ProtoAST->codegen()) {

            // output llvm IR
            fprintf(stderr, "Read extern:\n");
            FnIR->print(llvm::errs());
            fprintf(stderr, "\n");

            // add function to list of callable functions
            IR::FunctionProtos[ProtoAST->getName()] = std::move(ProtoAST);
        }
    } else {
        Lexer::getNextToken();
    }
}

static void HandleTopLevelExpression() {
    if (auto FnAST = Parser::ParseTopLevelExpr()) {
        if (auto * FnIR = FnAST->codegen()) {

            //output llvm IR
            fprintf(stderr, "Read top-level expression:\n");
            FnIR->print(llvm::errs());
            fprintf(stderr, "\n");

            // ressource allocation and gc
            auto RT = JIT::TheJIT->getMainJITDylib().createResourceTracker();

            // add finished module containing expression and create new module for future code
            auto TSM = llvm::orc::ThreadSafeModule(std::move(IR::Module), std::move(IR::Context));
            JIT::ExitOnErr(JIT::TheJIT->addModule(std::move(TSM), RT));
            IR::InitializeModuleAndPassManager();

            //execute expression
            auto ExprSymbol = JIT::ExitOnErr(JIT::TheJIT->lookup("__anon_expr"));
            double (*FP) () = ExprSymbol.getAddress().toPtr<double (*) ()>();
            fprintf(stderr, "Evaluated to %f\n\n", FP());

            // cleanup
            JIT::ExitOnErr(RT->remove());

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
                continue;
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
    // setup JIT backend to run on host
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    // initialize user interface
    fprintf(stderr, "ready> ");
    Lexer::getNextToken();

    // create JIT and create module for future code
    JIT::TheJIT = JIT::ExitOnErr(llvm::orc::KaleidoscopeJIT::Create());
    IR::InitializeModuleAndPassManager();

    // continue evalueting expressions till eof (Ctrl+D)
    MainLoop();

    return 0;
}
