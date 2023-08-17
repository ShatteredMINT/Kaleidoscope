clang++ -rdynamic -g -O3 main.cpp lexer.cpp parser.cpp log.cpp ast.cpp jit.cpp $(llvm-config --cxxflags --ldflags --system-libs --libs core orcjit native) -o build/main
