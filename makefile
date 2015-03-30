SOURCES:=$(shell find ./sources -name "*.cpp")
#SOURCES:=sources/clangapi.cpp
OBJ:=$(patsubst %.cpp,%.o,$(SOURCES))



TARGETNAME:=run
INCDIRS:=
LIBDIRS:=/usr/lib
LIBS:=clang llvm ncurses \
		$(patsubst lib%.a,%,$(shell ls /usr/lib/llvm-3.4/lib | grep "\.a"))
#	  clangAST clangFrontend clangLex clangSerialization clangDriver  \
		clangTooling clangParse clangSema clangAnalysis clangRewriteFrontend \
		clangRewriteCore clangEdit clangAST clangLex clangBasic

LIBS_= $(LIBS)  $(LIBS)

# $(shell llvm-config --cflags) 
CXXFLAGS:= -I/usr/lib/llvm-3.4/include -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -g -std=c++11 
LFLAGS:= $(shell llvm-config --ldflags)



CXX:=g++
LINKER:=g++
CXXFLAGS+=$(patsubst %,-I%,$(INCDIRS))
LFLAGS+=$(patsubst %,-L%,$(LIBDIRS)) $(patsubst %,-l%,$(LIBS_))  $(shell llvm-config --libs) -lpthread -ldl


all: $(OBJ)
	$(LINKER) $^ $(LFLAGS) -o $(TARGETNAME) 

$(OBJDIR)%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	rm $(OBJ) | true

rebuild: clean all
	echo rebuild

