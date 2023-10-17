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

static int TopCompare() {
    switch (Lexer::CurTok) {
        case tok_eof:
            return 1;
        case ';':
            Lexer::getNextToken(); // ignore top level smicolons
            return 0;
        case tok_def:
            HandleDefinition();
            break;
        case tok_extern:
            HandleExtern();
            break;
        default:
            HandleTopLevelExpression();
            break;

        return 0;
    }
}

static void UserLoop() {
    fprintf(stderr, "ready> ");
    Lexer::getNextToken();
    while (TopCompare() == 0) {
        fprintf(stderr, "ready> ");
    }
}

void printHelp() {
    printf("A simple kaleidoscope interpreter\n\n");
    printf("-i\t--include-file\t\tspecify a file with kaleidoscope code to include / execute before starting the console\n");
    printf("--help\tprint this help\n");
    printf("\n");
    printf("Author: ShatteredMINT\n\n");
    exit(0);
}

void fromFile(char * filename) {
    FILE * stream = fopen(filename, "r");

    if (stream)
        Lexer::stream = stream;
    else {
        fprintf(stderr, "ERROR: couldn't read file: %s\n defaulting to stdin", filename);
    }

    Lexer::getNextToken();
    while(TopCompare() == 0);

    Lexer::stream = stdin;
    fclose(stream);
}

void parseArgs(int argc, char ** argv) {

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case '-':
                    switch (argv[i][2]) {
                        case '\0':
                            return;
                        case 'h':
                            if (strcmp(argv[i], "--help") == 0)
                                printHelp();
                            break;
                        case 'i':
                            if (strcmp(argv[i], "--include-file") == 0)
                                fromFile(argv[++i]);
                            break;
                        default:
                            fprintf(stderr, "ERROR: invalid option: %s", argv[i]);
                            exit(0);
                    };
                    break;

                case '?':
                    printHelp();
                    break;

                case 'i':
                    fromFile(argv[++i]);
                    break;

                default:
                    fprintf(stderr, "ERROR: invalid option: %s", argv[i]);
                    exit(0);
            }
        } else {
            fprintf(stderr, "ERROR: invalid option: %s", argv[i]);
            exit;
        }
    }
}

int main(int argc, char ** argv) {
    // setup JIT backend to run on host
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    // create JIT and create module for future code
    JIT::TheJIT = JIT::ExitOnErr(llvm::orc::KaleidoscopeJIT::Create());
    IR::InitializeModuleAndPassManager();

    parseArgs(argc, argv);


    // continue evalueting expressions till eof (Ctrl+D)
    UserLoop();

    return 0;
}
