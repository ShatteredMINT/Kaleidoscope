clang++ -g -O3 main.cpp lexer.cpp parser.cpp log.cpp ast.cpp $(llvm-config --cxxflags --ldflags --system-libs --libs core) -o build/main
