REXGREP_BIN_INSTALL = /usr/bin
RLIB_SRCDIR = $(ROOT_DIR)/rlib
RVM_SRCDIR = $(ROOT_DIR)/rvm
RPA_SRCDIR = $(ROOT_DIR)/rpa
REX_SRCDIR = $(ROOT_DIR)/rex
REXGREP_SRCDIR = $(ROOT_DIR)/rexgrep
INCLUDE = -I$(ROOT_DIR) -I$(ROOT_DIR)/arch/unix/$(ARCHDIR) -I$(REXGREP_SRCDIR) -I$(REXGREP_SRCDIR)/unix
ifeq ($(OS), linux)
LDFLAGS += --static
endif


LIBS = -L$(RLIB_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(RVM_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(RPA_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(REX_SRCDIR)/build/unix/$(ARCHDIR)/out
LIBS += -lrpa -lrvm -lrex -lrlib -lpthread -lm

OBJECTS	= $(OUTDIR)/rexgrep.o \
	$(OUTDIR)/main.o \
	$(OUTDIR)/fsenum.o \
	$(OUTDIR)/rexgrepdep.o

REXGREP	= rexgrep


all : $(OUTDIR) $(OUTDIR)/$(REXGREP)

$(OUTDIR)/$(REXGREP) : $(OBJECTS)
	$(CC) -o $@ $^ $(LIBS) $(LDFLAGS)


$(OUTDIR)/%.o: $(REXGREP_SRCDIR)/%.c
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(REXGREP_SRCDIR)/$*.c $(INCLUDE)

$(OUTDIR)/%.o: $(REXGREP_SRCDIR)/unix/%.c
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(REXGREP_SRCDIR)/unix/$*.c $(INCLUDE)


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
	-rm -f $(OUTDIR)/$(REXGREP)

install:
	cp $(OUTDIR)/$(REXGREP) $(REXGREP_BIN_INSTALL)

uninstall:
	-rm -f $(REXGREP_BIN_INSTALL)/$(REXGREP)


