ROOT_DIR = ../..
include $(ROOT_DIR)/build/unix/config.mk

OUTDIR = bin
RLIB_SRCDIR = $(ROOT_DIR)/rlib
RVM_SRCDIR = $(ROOT_DIR)/rvm
RPA_SRCDIR = $(ROOT_DIR)/rpa

EXE_NAME = rpagrep
EXE_TARGET = $(OUTDIR)/$(EXE_NAME)

CFLAGS += -fPIC
CFLAGS += -I$(ROOT_DIR) -I.. -I../unix -I$(ROOT_DIR)/arch/unix -I$(RLIB_SRCDIR) -I$(RVM_SRCDIR)  -I$(RPA_SRCDIR)

LIBS = -L$(RLIB_SRCDIR)/unix/bin 
LIBS += -L$(RVM_SRCDIR)/unix/bin 
LIBS += -L$(RPA_SRCDIR)/unix/bin 
LIBS += -lrpa -lrvm -lrlib -lpthread -lm
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

