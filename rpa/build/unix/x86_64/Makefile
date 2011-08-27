SRCDIR = ../../../..
ARCHDIR = $(shell basename $(shell pwd))
OUTDIR = out
OS = $(shell uname | tr "[:upper:]" "[:lower:]")
INCLUDE = -I$(SRCDIR)/arch/unix/$(ARCHDIR)
MACH = -m64

include $(SRCDIR)/build/unix/config.mk

CFLAGS := $(MACH)
ifeq ($(BLDCFG), release)
CFLAGS += -fPIC -O3
else
CFLAGS += -fPIC -O0 -g -Wall 
endif

ifeq ($(CCBLD), yes)
CFLAGS += -fprofile-arcs -ftest-coverage
endif

CFLAGS += $(INCLUDE)
LDFLAGS := $(MACH)

include $(SRCDIR)/build/unix/include.mk
include ../rpa.mk
