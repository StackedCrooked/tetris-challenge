OPTFLAG=-O3
DBGFLAG=-g

CXX=g++
LANG=c++0x
FLAGS=--ansi -Wall --pedantic --std=$(LANG) -I. $(OPTFLAG) $(DBGFLAG) 
LDFLAGS=-lboost_unit_test_framework -lboost_thread
LIBHDRS=$(wildcard stm/*.hpp)
LIBSRCS=$(wildcard stm/*.cpp)
UNITTESTSRCS=$(wildcard tests/*.cpp)
PERFTESTSRCS=$(wildcard perftests/*.cpp)
LIBOBJS=$(addprefix obj/stm/, $(patsubst %.cpp, %.o, $(notdir $(LIBSRCS))))
UNITTESTOBJS=$(addprefix obj/tests/, $(patsubst %.cpp, %.o, $(notdir $(UNITTESTSRCS))))
PERFTESTOBJS=$(addprefix obj/perf/, $(patsubst %.cpp, %.o, $(notdir $(PERFTESTSRCS))))
INCLUDE=-I$(HOME)/catch
#-Id:/dev/lib/boost/include
#LIBS=-L/usr/lib

bin/tests: $(LIBOBJS) $(UNITTESTOBJS) $(LIBHDRS) bin
	$(CXX) $(LIBS) $(INCLUDE) $(FLAGS) $(LIBOBJS) $(UNITTESTOBJS) $(LDFLAGS) -o $@

bin/perf: $(LIBOBJS) $(PERFTESTOBJS) $(LIBHDRS) bin

bin/%-suite: obj/tests/%.o $(LIBOBJS) obj/tests/main.o $(LIBHDRS) bin
	$(CXX) $(LIBS) $(INCLUDE) $(FLAGS) $(LIBOBJS) $(LDFLAGS) $< obj/tests/main.o -o $@

obj/stm/%.o: stm/%.cpp obj/stm
	$(CXX) $(LIBS) $(INCLUDE) $(FLAGS) -c $< -o $@

obj/tests/%.o: tests/%.cpp obj/tests
	$(CXX) $(LIBS) $(INCLUDE) $(FLAGS) -c $< -o $@

obj/perf/%.o: perftests/%.cpp obj/perf
	$(CXX) $(LIBS) $(INCLUDE) $(FLAGS) -c $< -o $@

bin/:
	mkdir -p bin

obj/stm:
	mkdir -p obj/stm

obj/tests:
	mkdir -p obj/tests

obj/perf:
	mkdir -p obj/perf


clean: FORCE
	rm -rf obj
	rm -rf bin

FORCE:

