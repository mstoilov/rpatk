ARCHDIR = $(shell basename $(shell pwd))
OUTDIR = out
OS = $(shell uname | tr "[:upper:]" "[:lower:]")
ROOT_DIR = ../../../../..

include $(ROOT_DIR)/build/unix/config.mk

ifeq ($(BLDCFG), release)
CFLAGS = -O2
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
