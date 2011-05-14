ROBJECT_SRCDIR = $(SRCDIR)/robject
RLIB_SRCDIR = $(SRCDIR)/rlib
RVM_SRCDIR = $(SRCDIR)/rvm
RPA2_SRCDIR = $(SRCDIR)/rpa2
RJS_SRCDIR = $(SRCDIR)/rjs
TESTS_SRCDIR = $(SRCDIR)/testrjs
INCLUDE = -I$(SRCDIR)/arch/$(OS)/$(ARCHDIR) -I$(ROBJECT_SRCDIR) -I$(RLIB_SRCDIR) -I$(RVM_SRCDIR) -I$(RJS_SRCDIR) -I$(RPA2_SRCDIR) 

LIBS =  -L$(ROBJECT_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RLIB_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RVM_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RJS_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RPA2_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -lrjs -lrpa2 -lrvm -lrlib -lpthread -lm --static


TESTS   += $(OUTDIR)/rjs-simple


all : $(OUTDIR) $(TESTS)


$(OUTDIR)/%: $(TESTS_SRCDIR)/%.c
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$* $(TESTS_SRCDIR)/$*.c  $(LIBS) $(INCLUDE)


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

