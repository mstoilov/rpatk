ROOT_DIR = ../../../..
include $(ROOT_DIR)/build/unix/config.mk

OUTDIR = bin
TESTS_SRCDIR = ../..
RLIB_SRCDIR = $(ROOT_DIR)/rlib
REX_SRCDIR = $(ROOT_DIR)/rex
EXE_NAME = rpagrep
EXE_TARGET = $(OUTDIR)/$(EXE_NAME)

CFLAGS += -I$(ROOT_DIR) -I.. -I../unix -I$(ROOT_DIR)/arch/unix -I$(RLIB_SRCDIR) -I$(REX_SRCDIR)

LIBS = -L$(ROOT_DIR)/build/unix/bin 
LIBS += -lrex -lrlib -lpthread -lm
LDFLAGS += $(LIBS)

ifeq ($(OS), linux)
LDFLAGS += --static
endif

-include $(OUTDIR)/*.d

TESTS	+= $(OUTDIR)/main
TESTS	+= $(OUTDIR)/rexregex

all: $(TESTS)

$(OUTDIR)/%: $(TESTS_SRCDIR)/%.c Makefile | bin
	+ $(CC) $(CFLAGS) -MMD -MF $(@:%.o=%.d) -o $(OUTDIR)/$* $(TESTS_SRCDIR)/$*.c $(LDFLAGS) $(INCLUDE)

$(OUTDIR):
	mkdir $@		


clean:
	-rm -f *~
	-rm -rf $(OUTDIR)

