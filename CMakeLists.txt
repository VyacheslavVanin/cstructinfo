cmake_minimum_required(VERSION 3.1)
project(cstructinfo)
set (CMAKE_CXX_STANDARD 14)

find_package(LLVM REQUIRED)
find_package(Boost REQUIRED)

include(FindPkgConfig)
pkg_search_module(JSONCPP REQUIRED jsoncpp)

include_directories(${LLVM_INCLUDE_DIRS}
                    clanghelper/stdhelper
                    clanghelper
                    sources
                    ${JSONCPP_INCLUDE_DIRS}
                    .)

# Set Source files
file(GLOB_RECURSE SRCS sources/*.cpp clanghelper/*.cpp)
file(GLOB_RECURSE HDRS sources/*.h
                       sources/*.hpp
                       clanghelper/*.hpp
                       clanghelper/*.h)

add_executable(cstructinfo ${SRCS} ${HDRS})


link_directories(${LLVM_LIBRARY_DIRS})
find_library(CLANGAST clangAST HINTS ${LLVM_LIBRARY_DIRS})
find_library(CLANGFRONTEND clangFrontend HINTS ${LLVM_LIBRARY_DIRS})
find_library(CLANGLEX clangLex HINTS ${LLVM_LIBRARY_DIRS})
find_library(CLANGSERIALIZATION clangSerialization HINTS ${LLVM_LIBRARY_DIRS})
find_library(CLANGDRIVER clangDriver HINTS ${LLVM_LIBRARY_DIRS})
find_library(CLANGTOOLING clangTooling HINTS ${LLVM_LIBRARY_DIRS})
find_library(CLANGPARSE clangParse HINTS ${LLVM_LIBRARY_DIRS})
find_library(CLANGSEMA clangSema HINTS ${LLVM_LIBRARY_DIRS})
find_library(CLANGANALYSIS clangAnalysis HINTS ${LLVM_LIBRARY_DIRS})
find_library(CLANGREWRITEFRONTEND clangRewriteFrontend HINTS
                                                       ${LLVM_LIBRARY_DIRS})
find_library(CLANGEDIT clangEdit HINTS ${LLVM_LIBRARY_DIRS})
find_library(CLANGLEX clangLex HINTS ${LLVM_LIBRARY_DIRS})
find_library(CLANGBASIC clangBasic HINTS ${LLVM_LIBRARY_DIRS})
find_library(CLANG clang HINTS ${LLVM_LIBRARY_DIRS})

set(CLANG_LIBS ${CLANGFRONTEND} ${CLANGLEX} ${CLANGSERIALIZATION}
               ${CLANGDRIVER} ${CLANGTOOLING} ${CLANGPARSE} ${CLANGSEMA}
               ${CLANGANALYSIS} ${CLANGREWRITEFRONTEND} ${CLANGEDIT}
               ${CLANGLEX} ${CLANGBASIC} ${CLANG} ${CLANGAST})

target_link_libraries(cstructinfo ${CLANG_LIBS} ${CLANG_LIBS}
    LLVM LLVMSupport LLVMOption LLVMMC ${JSONCPP_LIBRARIES})
