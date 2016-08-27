SOURCES:=$(shell find ./sources -name "*.cpp")
OBJ:=$(patsubst %.cpp,%.o,$(SOURCES))


TARGETNAME:=run
INCDIRS:=$(shell llvm-config --includedir)
LIBDIRS:=/usr/lib/clang
LIBS:=clang \
	  clangAST clangFrontend clangLex clangSerialization clangDriver  \
		clangTooling clangParse clangSema clangAnalysis clangRewriteFrontend \
		clangEdit clangAST clangLex clangBasic

CXXFLAGS:= $(shell llvm-config --cflags) 
CXXFLAGS += -g -O0
#CXXFLAGS += -O2

LD:=$(CXX)
CXXFLAGS+=$(patsubst %,-I%,$(INCDIRS))

LLL:=-lLLVM

LIBSFLAGS:=$(shell llvm-config --libs) \
		   $(patsubst %,-l%,$(LIBS)) \
		   $(shell llvm-config --system-libs)
LFLAGS+= $(shell llvm-config --ldflags) \
		 $(patsubst %,-L%,$(LIBDIRS)) \
		 $(LIBSFLAGS) \
		 $(LLL)


all: $(OBJ)
	$(LD) $^ $(LFLAGS) -o $(TARGETNAME) 

$(OBJDIR)%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	rm $(OBJ) | true

rebuild: clean all
	echo rebuild

