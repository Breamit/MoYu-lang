cmake_minimum_required(VERSION 3.10)
project(es_interpreter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(es
    src/main.cpp
)

target_include_directories(es PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)


add_executable(es_test
    src/marco.h
    src/type.h
    src/lexer/token.h
    src/lexer/lexer.h
    src/lexer/token.cpp
    src/lexer/lexer.cpp
    src/test/tokenizer_test.cpp
)

add_executable(es_parser_test
    src/marco.h
    src/type.h
    src/lexer/token.h
    src/lexer/lexer.h
    src/lexer/token.cpp
    src/lexer/lexer.cpp
    src/ast/ast.hpp
    src/ast/parser.h
    src/ast/parser.cpp
    src/test/parser_test1.cpp
)

target_include_directories(es_parser_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


target_include_directories(es_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
