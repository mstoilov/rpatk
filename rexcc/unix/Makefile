ROOT_DIR = ../..
include $(ROOT_DIR)/unix/config.mk

OUTDIR = bin
RLIB_SRCDIR = $(ROOT_DIR)/rlib
REX_SRCDIR = $(ROOT_DIR)/rex
EXE_NAME = rexcc
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

all: $(EXE_TARGET)

$(OUTDIR)/%.o: %.c Makefile | $(OUTDIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(OUTDIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(EXE_TARGET): $(OBJECTS) | $(OUTDIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OUTDIR):
	mkdir $@		


clean:
	-rm -f *~
	-rm -rf $(OUTDIR)
