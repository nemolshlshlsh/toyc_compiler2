cmake_minimum_required(VERSION 3.15)
project(ToyC-Compiler VERSION 1.0.0)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)


FLEX_TARGET(ToyC_Lexer src/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
BISON_TARGET(ToyC_Parser src/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp)

ADD_FLEX_BISON_DEPENDENCY(ToyC_Lexer ToyC_Parser)


include_directories(src)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


set(SOURCES
    src/main.cpp
    src/ast/ast.cpp
    src/semantic/analyzer.cpp
    src/codegen/riscv.cpp
    src/utils/utils.cpp
    ${FLEX_ToyC_Lexer_OUTPUTS}
    ${BISON_ToyC_Parser_OUTPUTS}
)


add_executable(compiler ${SOURCES})
add_executable(toyc ${SOURCES})
target_compile_options(compiler PRIVATE -Wall -Wextra -O2)
target_compile_options(toyc PRIVATE -Wall -Wextra -O2)


set_source_files_properties(
    ${FLEX_ToyC_Lexer_OUTPUTS} ${BISON_ToyC_Parser_OUTPUTS}
    PROPERTIES COMPILE_FLAGS "-Wno-unused-function -Wno-unused-variable -Wno-sign-compare"
)


install(TARGETS compiler toyc
    RUNTIME DESTINATION bin
)


add_custom_target(quick_test
    COMMAND echo "int main() { return 42; }" | $<TARGET_FILE:compiler>
    DEPENDS compiler
    COMMENT "Quick compilation test"
)