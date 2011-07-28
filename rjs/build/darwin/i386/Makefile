SRCDIR = ../../../..
ARCHDIR = $(shell basename $(shell pwd))
OUTDIR = out
OS = $(shell uname | tr "[:upper:]" "[:lower:]")
INCLUDE = -I$(SRCDIR)/arch/$(OS)/$(ARCHDIR)
ELFARCH = elf32-i386
BINARCH = i386

CC = gcc
AR = ar
LD = ld
OC = objcopy

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


include ../rjs.mk
