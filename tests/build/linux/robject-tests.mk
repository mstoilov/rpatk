ROBJECT_SRCDIR = $(SRCDIR)/robject
RLIB_SRCDIR = $(SRCDIR)/rlib
TESTS_SRCDIR = $(SRCDIR)/tests
INCLUDE = -I$(SRCDIR)/arch/$(OS)/$(ARCHDIR) -I$(ROBJECT_SRCDIR) -I$(RLIB_SRCDIR)
LIBS =  -L$(ROBJECT_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RLIB_SRCDIR)/build/$(OS)/$(ARCHDIR)/out -lrlib -lpthread --static


TESTS	= \
	$(OUTDIR)/rlock-test \
	$(OUTDIR)/rarray-test \
	$(OUTDIR)/memalloc-test \
	
all : $(OUTDIR) $(TESTS)

$(OUTDIR)/%: $(TESTS_SRCDIR)/%.c
	$(CC) $(CFLAGS) -o $(OUTDIR)/$* $(TESTS_SRCDIR)/$*.c $(LIBS) $(INCLUDE)


$(OUTDIR):
	@mkdir $(OUTDIR)

distclean: clean
	@rm -f .depend
	@rm -rf $(OUTDIR)

clean:
	@rm -f $(TESTS)
	@rm -f *~
	@rm -f $(SRCDIR)/*~

