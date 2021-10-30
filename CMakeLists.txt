cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PROJECT_NAME rpal)
set(FILES src/main.cpp)
list(APPEND FILES src/utils.hpp src/utils.cpp)
list(APPEND FILES src/token.cpp src/token.hpp)
list(APPEND FILES src/ast.hpp src/ast.cpp)
list(APPEND FILES src/cse.hpp src/cse.cpp)

project(${PROJECT_NAME} VERSION 0.1)
add_executable(${PROJECT_NAME} ${FILES})
