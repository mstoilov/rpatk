#
# This makefile uses the rexcc compiler to build the DFA definition
# from the regular expressions defined in rexjsondfa.rexcc
# This means the 'rexcc' must be in the default search path, or
# the REXCC must be defined on the command line. For example to point to 
# the rexcc in the build directory (assuming the project was built for x86_64):
#
# make REXCC=../rexcc/build/unix/x86_64/out/rexcc
#

CC = $(PREFIX)gcc
CXX = $(PREFIX)g++
REXCC ?= rexcc
LDFLAGS = 

ifeq ($(USE),c)
PROJECT = testrexjson-c

$(PROJECT) : rexjsondfa.h rexjson.c
	$(CC) -o $@ rexjson.c $(LDFLAGS) -DREXJSON_TEST_MAIN -g -O0

else
PROJECT = testrexjson-cpp

$(PROJECT) : rexjsondfa.h rexjson++.cpp
	$(CXX) -o $@ rexjson++.cpp $(LDFLAGS) -DREXJSON_TEST_MAIN -g -O0
endif

clean:
	rm -f $(PROJECT) $(OBJECTS) $(DEPS) rexjsondfa.h

rexjsondfa.h : rexjsondfa.rexcc
	$(REXCC) rexjsondfa.rexcc -o rexjsondfa.h
