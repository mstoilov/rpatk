ROOT_DIR = ../../..
include $(ROOT_DIR)/build/unix/config.mk

OUTDIR = bin
TESTS_SRCDIR = ..
RLIB_SRCDIR = $(ROOT_DIR)/rlib
RVM_SRCDIR = $(ROOT_DIR)/rvm
RPA_SRCDIR = $(ROOT_DIR)/rpa

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

TESTS	+= $(OUTDIR)/funcarg-test
TESTS   += $(OUTDIR)/codegen-test
TESTS   += $(OUTDIR)/codemap-test
TESTS   += $(OUTDIR)/rlock-test
TESTS   += $(OUTDIR)/scope-test
TESTS   += $(OUTDIR)/rhash-test
TESTS   += $(OUTDIR)/rvm-test
TESTS   += $(OUTDIR)/loop-test
TESTS   += $(OUTDIR)/speed-test
TESTS   += $(OUTDIR)/memalloc-test
TESTS   += $(OUTDIR)/asm-add
TESTS   += $(OUTDIR)/asm-adds
TESTS   += $(OUTDIR)/asm-b
TESTS   += $(OUTDIR)/asm-bitops
TESTS   += $(OUTDIR)/asm-callback
TESTS   += $(OUTDIR)/asm-clz
TESTS   += $(OUTDIR)/asm-cmp
TESTS   += $(OUTDIR)/asm-div
TESTS   += $(OUTDIR)/asm-loadstore
TESTS   += $(OUTDIR)/asm-mul
TESTS   += $(OUTDIR)/asm-sbc
TESTS   += $(OUTDIR)/asm-shiftops
TESTS   += $(OUTDIR)/asm-stack
TESTS   += $(OUTDIR)/asm-bl

ETESTS   += $(OUTDIR)/asm-ecmp
ETESTS   += $(OUTDIR)/asm-esub
ETESTS   += $(OUTDIR)/asm-eadd
ETESTS   += $(OUTDIR)/rarray-test
ETESTS   += $(OUTDIR)/rcarray-test
ETESTS   += $(OUTDIR)/rharray-test
ETESTS   += $(OUTDIR)/rmap-test
ETESTS   += $(OUTDIR)/string-test
ETESTS   += $(OUTDIR)/opmap-test
ETESTS   += $(OUTDIR)/asm-cast

-include $(OUTDIR)/*.d

all: $(TESTS)

$(OUTDIR)/%: $(TESTS_SRCDIR)/%.c Makefile | bin
	+ $(CC) $(CFLAGS) -MMD -MF $(@:%.o=%.d) -o $(OUTDIR)/$* $(TESTS_SRCDIR)/$*.c $(LIBS) $(LDFLAGS) $(INCLUDE)

$(OUTDIR):
	mkdir $@		


clean:
	-rm -f *~
	-rm -rf $(OUTDIR)

