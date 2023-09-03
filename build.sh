clang++ -rdynamic -g -O3 main.cpp lexer.cpp parser.cpp ast.cpp jit.cpp ir.cpp $(llvm-config --cxxflags --ldflags --system-libs --libs core orcjit native) -o build/main
