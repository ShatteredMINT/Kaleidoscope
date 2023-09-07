clang++ -rdynamic -g -O3 src/main.cpp src/lexer.cpp src/parser.cpp src/ast.cpp src/jit.cpp src/ir.cpp $(llvm-config --cxxflags --ldflags --system-libs --libs core orcjit native) -o build/main
