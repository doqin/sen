add_library(Parser STATIC 
    src/AST.cpp
    src/parser.cpp
)

target_include_directories(Parser PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ../Lexer/include
)