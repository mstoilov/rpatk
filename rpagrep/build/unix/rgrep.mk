RPAGREP_BIN_INSTALL = ${RPATK_BIN_INSTALL}
RLIB_SRCDIR = $(ROOT_DIR)/rlib
RVM_SRCDIR = $(ROOT_DIR)/rvm
RPA_SRCDIR = $(ROOT_DIR)/rpa
RGREP_SRCDIR = $(ROOT_DIR)/rpagrep
INCLUDE = -I$(ROOT_DIR) -I$(ROOT_DIR)/arch/unix/$(ARCHDIR) -I$(RGREP_SRCDIR) -I$(RGREP_SRCDIR)/unix
ifeq ($(OS), linux)
LDFLAGS += --static
endif


LIBS = -L$(RLIB_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(RVM_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(RPA_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -lrpa -lrvm -lrlib -lpthread -lm

OBJECTS	= $(OUTDIR)/rpagrep.o $(OUTDIR)/main.o $(OUTDIR)/fsenum.o $(OUTDIR)/rpagrepdep.o $(OUTDIR)/rpagreputf.o
RPAGREP	= rpagrep


all : $(OUTDIR) $(OUTDIR)/$(RPAGREP)

$(OUTDIR)/$(RPAGREP) : $(OBJECTS)
	$(CC) -o $@ $^ $(LIBS) $(LDFLAGS)


$(OUTDIR)/%.o: $(RGREP_SRCDIR)/%.c
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(RGREP_SRCDIR)/$*.c $(INCLUDE)

$(OUTDIR)/%.o: $(RGREP_SRCDIR)/unix/%.c
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(RGREP_SRCDIR)/unix/$*.c $(INCLUDE)


$(OUTDIR):
	@mkdir $(OUTDIR)

distclean: clean
	-rm -f .depend
	-rm -rf $(OUTDIR)

clean:
	-rm -f $(TESTS)
	-rm -f *~
	-rm -f $(ROOT_DIR)/*~
	-rm -f $(OBJECTS)
	-rm -f $(OUTDIR)/$(RPAGREP)

install:
	cp $(OUTDIR)/$(RPAGREP) $(RPAGREP_BIN_INSTALL)

uninstall:
	-rm -f $(RPAGREP_BIN_INSTALL)/$(RPAGREP)


