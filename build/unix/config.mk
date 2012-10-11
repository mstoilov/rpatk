ifndef RPATK_BIN_INSTALL
RPATK_BIN_INSTALL = ${RPATK_INSTALL_PREFIX}/usr/bin
endif 

ifndef RPATK_LIB_INSTALL
RPATK_LIB_INSTALL = ${RPATK_INSTALL_PREFIX}/usr/lib
endif

ifndef RPATK_INC_INSTALL
RPATK_INC_INSTALL = ${RPATK_INSTALL_PREFIX}/usr/include/rpatk
endif

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
