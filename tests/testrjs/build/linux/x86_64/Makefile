ARCHDIR = x86_64
OUTDIR = out
OS = $(shell uname | tr "[:upper:]" "[:lower:]")
SRCDIR = ../../../../..


CC = gcc
AR = ar
ifeq ($(BLDCFG), release)
CFLAGS = -O3
else
ifeq ($(BLDCFG), profile)
CFLAGS = -O0 -pg
else
CFLAGS = -g -O0 -Wall 
endif
endif

ifeq ($(CCBLD), yes)
CFLAGS += -fprofile-arcs -ftest-coverage
endif

CFLAGS += $(MACH) $(INCLUDE)

CFLAGS := $(CFLAGS)
LDFLAGS = $(MACH)


include ../testrjs.mk
