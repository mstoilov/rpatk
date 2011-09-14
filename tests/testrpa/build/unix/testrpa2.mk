RLIB_SRCDIR = $(ROOT_DIR)/rlib
RVM_SRCDIR = $(ROOT_DIR)/rvm
RPA_SRCDIR = $(ROOT_DIR)/rpa
TESTS_SRCDIR = $(ROOT_DIR)/tests/testrpa
INCLUDE = -I$(ROOT_DIR) -I$(ROOT_DIR)/arch/unix/$(ARCHDIR)
ifeq ($(OS), linux)
LDFLAGS += --static
endif

LIBS = -L$(RLIB_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(RVM_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(RPA_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -lrpa -lrvm -lrlib -lpthread -lm


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


all : $(OUTDIR) $(TESTS)


$(OUTDIR)/%: $(TESTS_SRCDIR)/%.c
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$* $(TESTS_SRCDIR)/$*.c  -lrpa $(LIBS) $(LDFLAGS) $(INCLUDE)


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

