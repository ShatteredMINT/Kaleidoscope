# Kaleidoscope
This is my implementation of the Kaleidoscope language from the llvm tutorials

The major version will respond to the llvm tutorial chapter

**there is no sandbox or any other security mechanism**... this is technically exploitable... tho i don't see anybody using this for mission critical things

## Install
The only runtime dependency is the [llvm](https://llvm.org/docs/GettingStarted.html) base (>=v17)

cmake build instructions:
```
cmake -S . -B build && cmake --build build && cmake --install build --prefix .
```

## Usage
run the created binary in a terminal... **boom!** Kaleidoscope interpreter

```
bin/Kaleidoscope
```

currently supported:
- basic math operations (+, -, *, <)
- functions defined with syntax 'def function(arg1 arg2) code;' and called with 'function(1, 2);'
- variables declared with 'def var = 5.0;' or assigned with ' var = 2;'
- only supported datatypes are doubles (64 bit floating point values)
- libc is supported when declared as 'extern libcFunction(arg1 arg2);' and called as normal function
- extra functions 'putchard(x)' and 'printd(x)' for console output
- if with 'if COND then CODE1 else CODE2'
- for with 'for x = 1, x < 3[, 1.0] in CODE'
- custom unary and binary operators defined by naming a function "unary/binary + [operator]" (Ex: "unary!")
  - binary operators also need a precedence defined after the name (Ex: "binary| 10")
  - Full example for a greater than operator 'def binary> 10 (a b) b<a;'

## Example
if you need an example there is a mandelbrot.kal file that uses Kaleidoscope to display the mandelbrot set
