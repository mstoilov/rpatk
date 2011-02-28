RLIB_SRCDIR = $(SRCDIR)/rlib
RVM_SRCDIR = $(SRCDIR)/rvm
RPA2_SRCDIR = $(SRCDIR)/rpa2
TESTS_SRCDIR = $(SRCDIR)/testrpa2
INCLUDE = -I$(SRCDIR)/arch/$(OS)/$(ARCHDIR) -I$(RLIB_SRCDIR) -I$(RVM_SRCDIR) -I$(RPA2_SRCDIR)


LIBS = -L$(RLIB_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RVM_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RPA2_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -lrpa2 -lrvm -lrlib -lpthread -lm --static


TESTS	+= $(OUTDIR)/rpavm-matchchr
TESTS	+= $(OUTDIR)/rpavm-mnode
TESTS	+= $(OUTDIR)/rpacompiler-rule
TESTS	+= $(OUTDIR)/rpacompiler-exp

all : $(OUTDIR) $(TESTS)


$(OUTDIR)/%: $(TESTS_SRCDIR)/%.c
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$* $(TESTS_SRCDIR)/$*.c  -lrpa2 $(LIBS) $(INCLUDE)


$(OUTDIR)/%.o: $(TESTS_SRCDIR)/%.rpa
	$(LD) -r -b binary -o $(OUTDIR)/$*.o $(TESTS_SRCDIR)/$*.rpa


$(OUTDIR):
	@mkdir $(OUTDIR)

distclean: clean
	@rm -f .depend
	@rm -rf $(OUTDIR)

clean:
	@rm -f $(TESTS)
	@rm -f *~
	@rm -f $(SRCDIR)/*~

