ROBJECT_SRCDIR = $(SRCDIR)/robject
TESTS_SRCDIR = $(SRCDIR)/tests
INCLUDE = -I$(SRCDIR)/arch/$(OS)/$(ARCHDIR) -I$(ROBJECT_SRCDIR)
LIBS = -L$(ROBJECT_SRCDIR)/build/$(OS)/$(ARCHDIR)/out -lrobject --static



ROBJECT_OBJECTS =	\
	$(OUTDIR)/robject.o \


TESTS	= \
	$(OUTDIR)/robject-ver \

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

