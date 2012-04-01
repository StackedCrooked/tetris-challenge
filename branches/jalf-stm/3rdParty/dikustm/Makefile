ifdef dbg
DBGFLAG=-g
else
OPTFLAG=-O3
endif

LANG=c++98
#FLAGS=-Wall --ansi --pedantic --std=$(LANG) -I. $(OPTFLAG) $(DBGFLAG) -I/home/jalf/dev/boost_1_42_0 -L/home/jalf/dev/boost_1_42_0/stage/lib
FLAGS=-Wall --ansi --pedantic --std=$(LANG) -I. $(OPTFLAG) $(DBGFLAG)
LDFLAGS=-lboost_unit_test_framework -lboost_thread
LIBHDRS=$(wildcard stm/*.hpp)
LIBSRCS=$(wildcard stm/*.cpp)
UNITTESTSRCS=$(wildcard tests/*.cpp)
LIBOBJS=$(addprefix obj/stm/, $(patsubst %.cpp, %.o, $(notdir $(LIBSRCS))))
UNITTESTOBJS=$(addprefix obj/tests/, $(patsubst %.cpp, %.o, $(notdir $(UNITTESTSRCS))))

bin/tests: $(LIBOBJS) $(UNITTESTOBJS) $(LIBHDRS) bin
	g++ $(FLAGS) $(LDFLAGS) $(LIBOBJS) $(UNITTESTOBJS) -o $@

bin/%-suite: obj/tests/%.o $(LIBOBJS) obj/tests/main.o $(LIBHDRS) bin
	g++ $(FLAGS) $(LDFLAGS) $(LIBOBJS) $< obj/tests/main.o -o $@

obj/stm/%.o: stm/%.cpp obj/stm
	g++ $(FLAGS) -c $< -o $@

obj/tests/%.o: tests/%.cpp obj/tests
	g++ $(FLAGS) -c $< -o $@

bin/:
	mkdir bin

obj/:
	mkdir obj

obj/stm/: obj
	mkdir obj/stm

obj/tests/: obj
	mkdir obj/tests

clean: FORCE
	rm -rf obj
	rm -rf bin

FORCE:

