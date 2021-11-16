# RPAL Interpreter
CS3512 - Programming Languages
RPAL interpreter implemented according size rpal standards.

This is a complete RPAL interpreter. Which contains following parts,
- Scanner (Tokenize the source code)
- Parser (Parse AST)
- Analyzer (Convert AST to ST)
- CSE Machine (interpret the program)

# How to use

> Please Note that string literal syntax should use double quote
> instead of single quote.

Binary files are in "bin" directory.

Windows:
    rpal.exe <source-file-path>      => execute code
    rpal.exe -t <source-file-path>   => linear token list 
    rpal.exe -a <source-file-path>   => abstract syntax tree
    rpal.exe -s <source-file-path>   => standardized syntax tree

Linux:
    rpal <source-file-path>      => execute code
    rpal -t <source-file-path>   => linear token list
    rpal -a <source-file-path>   => abstract syntax tree
    rpal -s <source-file-path>   => standardized syntax tree


# How to build from source

## Requirements
- CMake 3.2+ [https://cmake.org/download/](https://cmake.org/download/)
- Make [https://gcc.gnu.org/](https://gcc.gnu.org/)
- C++ compiler (C++ STD 20) [https://gcc.gnu.org/](https://gcc.gnu.org/)

## Steps
1. Create directory named "build" `mkdir build` or `md build`
2. Go into that directory `cd build`
3. Type command `cmake ..`
4. Type command `make`
5. Binary file will be in the build directory `rpal.exe` (in windows)

# Source Code Description

#### `main.cpp`
- Contain entry point for the program.
- Handles command line arguments.

#### `token.hpp` and `token.cpp`
- Contains Scanner.

#### `ast.hpp` and `ast.cpp`
- Contains Parser.
- Contains algorithm to Standardize the AST.

#### `cse.hpp` and `cse.cpp`
- Contains the CSE machine

#### `utils.hpp` and `utils.cpp`
- Contains few utility functions
