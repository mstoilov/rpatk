SRCDIR = ../../../..
ARCHDIR = $(shell basename $(shell pwd))
OUTDIR = out
OS = $(shell uname | tr "[:upper:]" "[:lower:]")
INCLUDE = -I$(SRCDIR)/arch/$(OS)/$(ARCHDIR)

CC = gcc
AR = ar
LD = ld

ifeq ($(BLDCFG), release)
CFLAGS := -fPIC -O3
else 
ifeq ($(BLDCFG), profile)
CFLAGS := -fPIC -O0 -pg -Wall 
CFLAGS += -DR_DEBUG_MEMALLOC
else
CFLAGS := -fPIC -O0 -g -Wall 
CFLAGS += -DR_DEBUG_MEMALLOC
endif
endif

ifeq ($(CCBLD), yes)
CFLAGS += -fprofile-arcs -ftest-coverage
endif

CFLAGS += $(MACH) $(INCLUDE)
LDFLAGS := $(MACH)


include ../rlib.mk
