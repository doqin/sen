cmake_minimum_required(VERSION 3.15)
project(sen LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE FileReader Lexer)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
