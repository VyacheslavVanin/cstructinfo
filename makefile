SOURCES:=$(shell find ./sources -name "*.cpp")
OBJ:=$(patsubst %.cpp,%.o,$(SOURCES))


TARGETNAME:=cstructinfo
INCDIRS:=$(shell llvm-config --includedir)
LIBDIRS:=/usr/lib/clang
LIBS:=clang \
	  clangAST clangFrontend clangLex clangSerialization clangDriver  \
		clangTooling clangParse clangSema clangAnalysis clangRewriteFrontend \
		clangEdit clangAST clangLex clangBasic

CXXFLAGS:= $(shell llvm-config --cflags) 
CXXFLAGS += -std=c++14 -g -O0
#CXXFLAGS += -O2

LD:=$(CXX)
CXXFLAGS+=$(patsubst %,-I%,$(INCDIRS))

# Commented cause of bug \
#   "CommandLine Error: Option 'xcore-max-threads' registered more than once!"
#LLVMLIB:= $(shell llvm-config --libs)
LLVMLIBLIST:= LLVM LLVMSupport LLVMOption LLVMMC
LLVMLIB:= $(patsubst %,-l%,$(LLVMLIBLIST))

LIBSFLAGS:=$(patsubst %,-l%,$(LIBS)) \
		   $(LLVMLIB) \
		   $(shell llvm-config --system-libs)
LFLAGS+= $(shell llvm-config --ldflags) \
		 $(patsubst %,-L%,$(LIBDIRS)) \
		 $(LIBSFLAGS)


all: $(OBJ)
	$(LD) $^ $(LFLAGS) -o $(TARGETNAME) 

$(OBJDIR)%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	rm $(OBJ) | true

rebuild: clean all
	echo rebuild

