ROBJECT_SRCDIR = $(SRCDIR)/robject
RLIB_SRCDIR = $(SRCDIR)/rlib
RVM_SRCDIR = $(SRCDIR)/rvm
RPA_SRCDIR = $(SRCDIR)/rpa
RJS_SRCDIR = $(SRCDIR)/rjs
TESTS_SRCDIR = $(SRCDIR)/tests/testrjs
INCLUDE = -I$(SRCDIR) -I$(SRCDIR)/arch/$(OS)/$(ARCHDIR)

LIBS =  -L$(ROBJECT_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RLIB_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RVM_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RJS_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RPA_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -lrjs -lrpa -lrvm -lrlib -lpthread -lm --static


TESTS   += $(OUTDIR)/rjs-simple
TESTS   += $(OUTDIR)/rjs-args


all : $(OUTDIR) $(TESTS)


$(OUTDIR)/%: $(TESTS_SRCDIR)/%.c
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$* $(TESTS_SRCDIR)/$*.c  $(LIBS) $(INCLUDE)


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
	-rm -f $(SRCDIR)/*~

