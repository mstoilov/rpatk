ROOT_DIR = ../../..
ARCHDIR = $(shell basename $(shell pwd))
OUTDIR = out
OS = $(shell uname | tr "[:upper:]" "[:lower:]")
INCLUDE = -I$(ROOT_DIR)/arch/unix/$(ARCHDIR)

include $(ROOT_DIR)/build/unix/config.mk

ifeq ($(BLDCFG), release)
CFLAGS := -fPIC -O2
else
CFLAGS := -fPIC -O0 -g -Wall 
endif

ifeq ($(CCBLD), yes)
CFLAGS += -fprofile-arcs -ftest-coverage
endif

CFLAGS += $(MACH) $(INCLUDE)
LDFLAGS := $(MACH)

include ../template.mk
