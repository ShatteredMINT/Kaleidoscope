#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ast.h"
#include "lexer.h"
#include "parser.h"

//===-----------------------------------------
// DRIVER
//===-----------------------------------------

static void HandleDefinition() {
    if (Parser::ParseDefinition()) {
        fprintf(stderr, "Parsed a function definition \n");
    } else {
        Lexer::getNextToken();
    }
}

static void HandleExtern() {
    if (Parser::ParseExtern()) {
        fprintf(stderr, "Parsed an extern\n");
    } else {
        Lexer::getNextToken();
    }
}

static void HandleTopLevelExpression() {
    if (Parser::ParseTopLevelExpr()) {
        fprintf(stderr, "Parsed a top-level expr \n");
    } else {
        Lexer::getNextToken();
    }
}

static void MainLoop() {
    while (true) {
        fprintf(stderr, "ready> ");
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
    }
}

int main() {
    Parser::BinopPrecedence['<'] = 10;
    Parser::BinopPrecedence['+'] = 20;
    Parser::BinopPrecedence['-'] = 20;
    Parser::BinopPrecedence['*'] = 40;

    fprintf(stderr, "ready> ");
    Lexer::getNextToken();

    MainLoop();

    return 0;
}
