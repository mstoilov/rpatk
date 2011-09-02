RTK_LIB_INSTALL=/usr/lib
RPATK_INC_INSTALL=/usr/include/rpatk
ARCHDIR = $(shell basename $(shell pwd))
OS = $(shell uname | tr "[:upper:]" "[:lower:]")

CC = gcc
LD = ld
AR = ar
CPP = g++
OC = objcopy
