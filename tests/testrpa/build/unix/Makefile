ROOT_DIR = ../../../..
include $(ROOT_DIR)/build/unix/config.mk

OUTDIR = bin
TESTS_SRCDIR = ../..
RLIB_SRCDIR = $(ROOT_DIR)/rlib
RVM_SRCDIR = $(ROOT_DIR)/rvm
RPA_SRCDIR = $(ROOT_DIR)/rpa

CFLAGS += -I$(ROOT_DIR) -I.. -I../unix -I$(ROOT_DIR)/arch/unix -I$(RLIB_SRCDIR) -I$(RVM_SRCDIR)  -I$(RPA_SRCDIR)


ifeq ($(DEBUG), no)
CFLAGS += -O2
else
CFLAGS += -O0 -g
endif

ifeq ($(CCBLD), yes)
CFLAGS += -fprofile-arcs -ftest-coverage
endif

LIBS += -L$(ROOT_DIR)/build/unix/bin 
LIBS += -lrpa -lrvm -lrlib -lpthread -lm
LDFLAGS += $(LIBS)

OBJECTS += $(addprefix $(OUTDIR)/, $(patsubst %.c,%.o,$(notdir $(wildcard ../*.c))))
vpath %.c ../

OBJECTS += $(addprefix $(OUTDIR)/, $(patsubst %.c,%.o,$(notdir $(wildcard *.c))))
vpath %.c .

TESTS	+= $(OUTDIR)/rpavm-matchchr
TESTS	+= $(OUTDIR)/rpavm-matchrng
TESTS	+= $(OUTDIR)/rpavm-mnode
TESTS	+= $(OUTDIR)/rpavm-ref
;TESTS	+= $(OUTDIR)/rpacompiler-ruleloop
;TESTS	+= $(OUTDIR)/rpacompiler-ruleloopcls
TESTS	+= $(OUTDIR)/rpacompiler-rulerec
TESTS	+= $(OUTDIR)/rpacompiler-rulealtrec
TESTS	+= $(OUTDIR)/rpacompiler-rule
TESTS	+= $(OUTDIR)/rpacompiler-exp
TESTS	+= $(OUTDIR)/rpacompiler-notexp
TESTS	+= $(OUTDIR)/rpacompiler-class
TESTS	+= $(OUTDIR)/rpacompiler-altexp
TESTS	+= $(OUTDIR)/rpacompiler-minusexp
TESTS	+= $(OUTDIR)/rpaparser-test
TESTS	+= $(OUTDIR)/postfix


all: $(TESTS)

$(OUTDIR)/%: $(TESTS_SRCDIR)/%.c Makefile | bin
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$* $(TESTS_SRCDIR)/$*.c $(LIBS) $(LDFLAGS) $(INCLUDE)

$(OUTDIR):
	mkdir $@		


clean:
	-rm -f *~
	-rm -rf $(OUTDIR)

