REXCC_BIN_INSTALL = /usr/bin
RLIB_SRCDIR = $(ROOT_DIR)/rlib
RVM_SRCDIR = $(ROOT_DIR)/rvm
RPA_SRCDIR = $(ROOT_DIR)/rpa
REX_SRCDIR = $(ROOT_DIR)/rex
REXCC_SRCDIR = $(ROOT_DIR)/rexcc
INCLUDE = -I$(ROOT_DIR) -I$(ROOT_DIR)/arch/unix/$(ARCHDIR) -I$(REXCC_SRCDIR) -I$(REXCC_SRCDIR)/unix
ifeq ($(OS), linux)
LDFLAGS += --static
endif


LIBS = -L$(RLIB_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(RVM_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(RPA_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(REX_SRCDIR)/build/unix/$(ARCHDIR)/out
LIBS += -lrpa -lrvm -lrex -lrlib -lpthread -lm

OBJECTS	= $(OUTDIR)/rexcc.o \
	$(OUTDIR)/rexccdep.o \
	$(OUTDIR)/main.o

REXCC	= rexcc


all : $(OUTDIR) $(OUTDIR)/$(REXCC)

$(OUTDIR)/$(REXCC) : $(OBJECTS)
	$(CC) -o $@ $^ $(LIBS) $(LDFLAGS)


$(OUTDIR)/%.o: $(REXCC_SRCDIR)/%.c
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(REXCC_SRCDIR)/$*.c $(INCLUDE)

$(OUTDIR)/%.o: $(REXCC_SRCDIR)/unix/%.c
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(REXCC_SRCDIR)/unix/$*.c $(INCLUDE)


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
	-rm -f $(OUTDIR)/$(REXCC)

install:
	cp $(OUTDIR)/$(REXCC) $(REXCC_BIN_INSTALL)

uninstall:
	-rm -f $(REXCC_BIN_INSTALL)/$(REXCC)


