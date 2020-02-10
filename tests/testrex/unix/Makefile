ROOT_DIR = ../../..
include $(ROOT_DIR)/unix/config.mk

OUTDIR = bin
TESTS_SRCDIR = ..
RLIB_SRCDIR = $(ROOT_DIR)/rlib
REX_SRCDIR = $(ROOT_DIR)/rex
EXE_NAME = rpagrep
EXE_TARGET = $(OUTDIR)/$(EXE_NAME)

CFLAGS += -I$(ROOT_DIR) -I.. -I../unix -I$(ROOT_DIR)/arch/unix -I$(RLIB_SRCDIR) -I$(REX_SRCDIR)

LIBS = -L$(REX_SRCDIR)/unix/bin 
LIBS += -L$(RLIB_SRCDIR)/unix/bin 
LIBS += -lrex -lrlib -lpthread -lm
LDFLAGS += $(LIBS)

ifeq ($(OS), linux)
LDFLAGS += --static
endif


TESTS	+= $(OUTDIR)/main
TESTS	+= $(OUTDIR)/rexregex

all: $(TESTS)

$(OUTDIR)/%: $(TESTS_SRCDIR)/%.c Makefile | bin
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$* $(TESTS_SRCDIR)/$*.c $(LDFLAGS) $(INCLUDE)

$(OUTDIR):
	mkdir $@		


clean:
	-rm -f *~
	-rm -rf $(OUTDIR)

