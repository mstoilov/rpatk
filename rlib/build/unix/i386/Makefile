ROOT_DIR = ../../../..
include $(ROOT_DIR)/build/unix/config.mk

INCLUDE = -I$(ROOT_DIR)/arch/unix/$(ARCHDIR)
OUTDIR = out
MACH = -m32

CFLAGS := $(MACH)
ifeq ($(BLDCFG), release)
CFLAGS += -fPIC -O3
else 
ifeq ($(BLDCFG), profile)
CFLAGS += -fPIC -O0 -pg -Wall
CFLAGS += -DR_DEBUG_MEMALLOC
else
CFLAGS += -fPIC -O0 -g -Wall
CFLAGS += -DR_DEBUG_MEMALLOC
endif
endif

ifeq ($(CCBLD), yes)
CFLAGS += -fprofile-arcs -ftest-coverage
endif

CFLAGS +=  $(INCLUDE)
LDFLAGS := $(MACH)


include ../rlib.mk
