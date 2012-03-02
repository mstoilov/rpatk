RLIB_SRCDIR = $(ROOT_DIR)/rlib
REX_SRCDIR = $(ROOT_DIR)/rex
TESTS_SRCDIR = $(ROOT_DIR)/tests/testrex
INCLUDE = -I$(ROOT_DIR) -I$(ROOT_DIR)/arch/unix/$(ARCHDIR)
ifeq ($(OS), linux)
LDFLAGS += --static
endif

LIBS = -L$(RLIB_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(REX_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -lrex -lrlib -lm


TESTS	+= $(OUTDIR)/rexregex


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

