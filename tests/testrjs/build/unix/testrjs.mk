ROBJECT_SRCDIR = $(ROOT_DIR)/robject
RLIB_SRCDIR = $(ROOT_DIR)/rlib
RVM_SRCDIR = $(ROOT_DIR)/rvm
RPA_SRCDIR = $(ROOT_DIR)/rpa
RJS_SRCDIR = $(ROOT_DIR)/rjs
TESTS_SRCDIR = $(ROOT_DIR)/tests/testrjs
INCLUDE = -I$(ROOT_DIR) -I$(ROOT_DIR)/arch/unix/$(ARCHDIR)
ifeq ($(OS), linux)
LDFLAGS += --static
endif

LIBS =  -L$(ROBJECT_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(RLIB_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(RVM_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(RJS_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(RPA_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -lrjs -lrpa -lrvm -lrlib -lpthread -lm


TESTS   += $(OUTDIR)/rjs-simple
TESTS   += $(OUTDIR)/rjs-args


all : $(OUTDIR) $(TESTS)


$(OUTDIR)/%: $(TESTS_SRCDIR)/%.c
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$* $(TESTS_SRCDIR)/$*.c  $(LIBS) $(LDFLAGS) $(INCLUDE)


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

