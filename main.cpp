#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

#include "ast.h"
#include "lexer.h"
#include "parser.h"

//===-----------------------------------------
// DRIVER
//===-----------------------------------------

static void InitializeModule() {
    AST::Context = std::make_unique<llvm::LLVMContext>();
    AST::Module = std::make_unique<llvm::Module>("My cool JIT", *AST::Context);
    AST::Builder = std::make_unique<llvm::IRBuilder<>>(*AST::Context);
}

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
    Parser::BinopPrecedence['<'] = 10;
    Parser::BinopPrecedence['+'] = 20;
    Parser::BinopPrecedence['-'] = 20;
    Parser::BinopPrecedence['*'] = 40;

    fprintf(stderr, "ready> ");
    Lexer::getNextToken();

    InitializeModule();

    MainLoop();

    AST::Module->print(llvm::errs(), nullptr);

    return 0;
}
