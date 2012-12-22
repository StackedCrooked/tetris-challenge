OPTFLAG=-O3
DBGFLAG=-g

# note, -isystem can be used to specify a system header dir, to silence boost warnigns

CXX=g++-mp-4.6
LANG=c++0x
FLAGS=--ansi -Wall --pedantic -Wno-delete-non-virtual-dtor -msse2 --std=$(LANG) -I. $(OPTFLAG) $(DBGFLAG)
LIB= -lboost_thread-mt

ifeq ($(CXX),clang++)
FLAGS+=-stdlib=libc++
endif

INCLUDE=-I$(HOME)/dev/3rdparty/ -I/opt/local/include
LIBPATH=-L/opt/local/lib

COMPILE=$(CXX) -std=$(LANG) $(FLAGS) $(INCLUDE)
LINK=$(COMPILE) $(LIBPATH) $(LIB)

DIRS:=tests perftests perftests/internal
SRC:=$(wildcard $(DIRS:%=%/*.cpp))
OBJ:=$(SRC:%.cpp=obj/%.o)

# need to fix deps on test/perftest headers

bin/tests: $(filter obj/tests/%,$(OBJ))
	mkdir -p bin
	$(LINK) $+ -o $@

bin/perftests: $(filter obj/perftests/%,$(OBJ))
	mkdir -p bin
	$(LINK) $+ -o $@

obj/%.o: %.cpp stm/*.hpp
	mkdir -p $(@D)
	$(COMPILE) -c $< -o $@

$(DIRS:%=obj/%):
	mkdir -p $(@D)

clean: FORCE
	rm -rf obj
	rm -rf bin

FORCE: ;

