add_library(FileReader STATIC src/ReadFile.cpp)

target_include_directories(FileReader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
