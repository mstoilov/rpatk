RTK_LIB_INSTALL=/usr/lib
RPATK_INC_INSTALL=/usr/include/rpatk
ARCHDIR = $(shell basename $(shell pwd))
OS = $(shell uname | tr "[:upper:]" "[:lower:]")

ifndef CC
CC = gcc
endif

ifndef LD
LD = ld
endif

ifndef AR
AR = ar
endif

ifndef CPP
CPP = g++
endif

ifndef OBJCOPY
OBJCOPY = objcopy
endif
