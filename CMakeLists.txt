cmake_minimum_required(VERSION 3.1)
project(cstructinfo)
set (CMAKE_CXX_STANDARD 14)

find_package(LLVM REQUIRED)

include_directories(${LLVM_INCLUDE_DIRS} sources .)

# Set Source files
file(GLOB_RECURSE SRCS sources/*.cpp)
file(GLOB_RECURSE HDRS sources/*.h sources/*.hpp)

add_executable(cstructinfo ${SRCS} ${HDRS})

set(CLANG_LIBS
    clangAST clangFrontend clangLex clangSerialization clangDriver
    clangTooling clangParse clangSema clangAnalysis clangRewriteFrontend
    clangEdit clangAST clangLex clangBasic clang)

target_link_libraries(cstructinfo ${CLANG_LIBS} ${CLANG_LIBS}
    LLVM LLVMSupport LLVMOption LLVMMC)
