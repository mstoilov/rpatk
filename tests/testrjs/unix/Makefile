ROOT_DIR = ../../..
include $(ROOT_DIR)/build/unix/config.mk

OUTDIR = bin
TESTS_SRCDIR = ..
RLIB_SRCDIR = $(ROOT_DIR)/rlib
RVM_SRCDIR = $(ROOT_DIR)/rvm
RPA_SRCDIR = $(ROOT_DIR)/rpa
RJS_SRCDIR = $(ROOT_DIR)/rjs

CFLAGS += -I$(ROOT_DIR) -I.. -I../unix -I$(ROOT_DIR)/arch/unix -I$(RLIB_SRCDIR) -I$(RVM_SRCDIR) -I$(RPA_SRCDIR) -I$(RJS_SRCDIR)


ifeq ($(DEBUG), no)
CFLAGS += -O2
else
CFLAGS += -O0 -g
endif

ifeq ($(CCBLD), yes)
CFLAGS += -fprofile-arcs -ftest-coverage
endif

LIBS = -L$(RLIB_SRCDIR)/unix/bin 
LIBS += -L$(RVM_SRCDIR)/unix/bin 
LIBS += -L$(RPA_SRCDIR)/unix/bin 
LIBS += -L$(RJS_SRCDIR)/unix/bin 
LIBS += -lrjs -lrpa -lrvm -lrlib -lpthread -lm
LDFLAGS += $(LIBS)

OBJECTS += $(addprefix $(OUTDIR)/, $(patsubst %.c,%.o,$(notdir $(wildcard ../*.c))))
vpath %.c ../

OBJECTS += $(addprefix $(OUTDIR)/, $(patsubst %.c,%.o,$(notdir $(wildcard *.c))))
vpath %.c .

TESTS   += $(OUTDIR)/rjs-simple
TESTS   += $(OUTDIR)/rjs-args


all: $(TESTS)

$(OUTDIR)/%: $(TESTS_SRCDIR)/%.c Makefile | bin
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$* $(TESTS_SRCDIR)/$*.c $(LIBS) $(LDFLAGS) $(INCLUDE)

$(OUTDIR)/%.o: $(TESTS_SRCDIR)/%.rpa
	$(LD) -r -b binary -o $(OUTDIR)/$*.o $(TESTS_SRCDIR)/$*.rpa


$(OUTDIR):
	mkdir $@		


clean:
	-rm -f *~
	-rm -rf $(OUTDIR)

