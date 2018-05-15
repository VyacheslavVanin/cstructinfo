cmake_minimum_required(VERSION 3.1)
project(cstructinfo)
set (CMAKE_CXX_STANDARD 14)

find_package(LLVM REQUIRED)
include(FindPkgConfig)
pkg_search_module(JSONCPP REQUIRED jsoncpp)

include_directories(${LLVM_INCLUDE_DIRS}
                    clanghelper/stdhelper
                    clanghelper
                    sources
                    .)

# Set Source files
file(GLOB_RECURSE SRCS sources/*.cpp clanghelper/*.cpp)
file(GLOB_RECURSE HDRS sources/*.h
                       sources/*.hpp
                       clanghelper/*.hpp
                       clanghelper/*.h)

add_executable(cstructinfo ${SRCS} ${HDRS})

set(CLANG_LIBS
    clangAST clangFrontend clangLex clangSerialization clangDriver
    clangTooling clangParse clangSema clangAnalysis clangRewriteFrontend
    clangEdit clangAST clangLex clangBasic clang)

target_link_libraries(cstructinfo ${CLANG_LIBS} ${CLANG_LIBS}
    LLVM LLVMSupport LLVMOption LLVMMC ${JSONCPP_LIBRARIES})
