ROOT_DIR = ../../../..
ARCHDIR = $(shell basename $(shell pwd))
OUTDIR = out
OS = $(shell uname | tr "[:upper:]" "[:lower:]")
INCLUDE = -I$(ROOT_DIR)/arch/unix/$(ARCHDIR)
MACH = -m64
ELFARCH = elf64-x86-64
BINARCH = i386:x86-64
OCFLAGS_TXT = --input binary --output $(ELFARCH) --binary-architecture $(BINARCH)

include $(ROOT_DIR)/build/unix/config.mk

ifeq ($(BLDCFG), release)
CFLAGS := $(MACH) -fPIC -O3
else
CFLAGS := $(MACH) -fPIC -O0 -g -Wall 
endif

ifeq ($(CCBLD), yes)
CFLAGS += -fprofile-arcs -ftest-coverage
endif

CFLAGS += $(INCLUDE)
LDFLAGS := $(MACH)


include ../rjs.mk
