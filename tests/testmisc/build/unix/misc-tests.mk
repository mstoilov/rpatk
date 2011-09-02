ROBJECT_SRCDIR = $(ROOT_DIR)/robject
RLIB_SRCDIR = $(ROOT_DIR)/rlib
RVM_SRCDIR = $(ROOT_DIR)/rvm
RPA_SRCDIR = $(ROOT_DIR)/rpa
RAST_SRCDIR = $(ROOT_DIR)/rast
TESTS_SRCDIR = $(ROOT_DIR)/tests/testmisc
INCLUDE = -I$(ROOT_DIR) -I$(ROOT_DIR)/arch/unix/$(ARCHDIR) -I$(RPA_SRCDIR)
ifeq ($(OS), linux)
LDFLAGS += --static
endif

LIBS = -L$(RLIB_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(RVM_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(RPA_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -lrpa -lrvm -lrlib -lpthread -lm


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

all : $(OUTDIR) $(TESTS)

$(OUTDIR)/%: $(TESTS_SRCDIR)/%.c
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$* $(TESTS_SRCDIR)/$*.c $(LIBS) $(LDFLAGS) $(INCLUDE)


$(OUTDIR)/%.o: $(TESTS_SRCDIR)/%.rpa
	$(LD) -r -b binary -o $(OUTDIR)/$*.o $(TESTS_SRCDIR)/$*.rpa


$(OUTDIR):
	@mkdir $(OUTDIR)

distclean: clean
	-rm -f .depend
	-rm -rf $(OUTDIR)

clean:
	-rm -f $(TESTS)
	-rm -f *~
	-rm -f $(ROOT_DIR)/*~

