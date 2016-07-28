SOURCES:=$(shell find ./sources -name "*.cpp")
OBJ:=$(patsubst %.cpp,%.o,$(SOURCES))


LLVM_VERSION:=3.5

TARGETNAME:=run
INCDIRS:=
LIBDIRS:=/usr/lib /usr/lib/llvm-$(LLVM_VERSION)/lib
LIBS:=clang llvm ncursesw tinfo\
		$(patsubst lib%.a,%,$(shell ls /usr/lib/llvm-$(LLVM_VERSION)/lib | grep "\.a")) 
#	  clangAST clangFrontend clangLex clangSerialization clangDriver  \
		clangTooling clangParse clangSema clangAnalysis clangRewriteFrontend \
		clangRewriteCore clangEdit clangAST clangLex clangBasic

LIBS_:= $(LIBS)  $(LIBS)

# $(shell llvm-config --cflags) 
CXXFLAGS:= -I/usr/lib/llvm-$(LLVM_VERSION)/include -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -std=c++11
CXXFLAGS += -g -O0
#CXXFLAGS += -O2
#LFLAGS:= $(shell llvm-config --ldflags)  $(shell llvm-config --libs) 


#CC="usr/bin/clang" 
#CXX="/usr/bin/clang++"
LD:=$(CXX)
CXXFLAGS+=$(patsubst %,-I%,$(INCDIRS))
LFLAGS+=$(patsubst %,-L%,$(LIBDIRS)) $(patsubst %,-l%,$(LIBS_)) -lpthread -ldl -lz -lLTO --static


all: $(OBJ)
	$(LD) $^ $(LFLAGS) -o $(TARGETNAME) 

$(OBJDIR)%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	rm $(OBJ) | true

rebuild: clean all
	echo rebuild

