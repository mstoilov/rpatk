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
CFLAGS := -fPIC -O0 -g -Wall 
endif

ifeq ($(CCBLD), yes)
CFLAGS += -fprofile-arcs -ftest-coverage
endif

CFLAGS += $(MACH) $(INCLUDE)
LDFLAGS := $(MACH)

include $(SRCDIR)/build/$(OS)/include.mk
include ../rpa.mk
