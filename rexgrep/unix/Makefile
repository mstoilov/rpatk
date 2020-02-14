ROOT_DIR = ../..
include $(ROOT_DIR)/build/unix/config.mk

OUTDIR = bin
RLIB_SRCDIR = $(ROOT_DIR)/rlib
REX_SRCDIR = $(ROOT_DIR)/rex
EXE_NAME = rexgrep
EXE_TARGET = $(OUTDIR)/$(EXE_NAME)

CFLAGS += -fPIC
CFLAGS += -I$(ROOT_DIR) -I.. -I../unix -I$(ROOT_DIR)/arch/unix -I$(RLIB_SRCDIR) -I$(REX_SRCDIR)

LIBS = -L$(RLIB_SRCDIR)/unix/bin
LIBS += -L$(REX_SRCDIR)/unix/bin
LIBS += -lrex -lrlib -lpthread -lm
LDFLAGS += $(LIBS)

OBJECTS += $(addprefix $(OUTDIR)/, $(patsubst %.c,%.o,$(notdir $(wildcard ../*.c))))
vpath %.c ../

OBJECTS += $(addprefix $(OUTDIR)/, $(patsubst %.c,%.o,$(notdir $(wildcard *.c))))
vpath %.c .

-include $(OUTDIR)/*.d

all: $(EXE_TARGET)

$(OUTDIR)/%.o: %.c Makefile | $(OUTDIR)
	$(CC) -c $(CFLAGS) -MMD -MF $(@:%.o=%.d) $< -o $@

$(EXE_TARGET): $(OBJECTS) | $(OUTDIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OUTDIR):
	mkdir $@		


clean:
	-rm -f *~
	-rm -rf $(OUTDIR)
