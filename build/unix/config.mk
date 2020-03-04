ifndef RPATK_BIN_INSTALL
RPATK_BIN_INSTALL = ${RPATK_INSTALL_PREFIX}/usr/bin
endif 

ifndef RPATK_LIB_INSTALL
RPATK_LIB_INSTALL = ${RPATK_INSTALL_PREFIX}/usr/lib/rpatk
endif

ifndef RPATK_INC_INSTALL
RPATK_INC_INSTALL = ${RPATK_INSTALL_PREFIX}/usr/include/rpatk
endif

DEBUG ?= yes
ARCH ?= x86_64
OS = $(shell uname | tr "[:upper:]" "[:lower:]")

ifeq ($(DEBUG), no)
CFLAGS += -O2
else
CFLAGS += -O0 -ggdb -DR_DEBUG_MEMALLOC
LDFLAGS += -ggdb
endif

ifeq ($(CCBLD), yes)
CFLAGS += -fprofile-arcs -ftest-coverage
endif

CFLAGS += -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-implicit-fallthrough

# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CPP = $(GCC_PATH)/$(PREFIX)g++
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
AR = $(GCC_PATH)/$(PREFIX)ar
LD = $(GCC_PATH)/$(PREFIX)ld
OBJCOPY = $(GCC_PATH)/$(PREFIX)objcopy
else
CPP = $(PREFIX)g++
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
AR = $(PREFIX)ar
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy
endif

