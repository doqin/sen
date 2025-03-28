add_library(Lexer STATIC 
    src/lexer.cpp
    src/token.cpp
)

target_include_directories(Lexer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
