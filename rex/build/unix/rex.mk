REX_SRCDIR = $(ROOT_DIR)/rex
REX_EXE = $(OUTDIR)/rex
REX_OBJECTS =	\
	$(OUTDIR)/main.o \
	$(OUTDIR)/rexdb.o \
	$(OUTDIR)/rexcompiler.o \
	$(OUTDIR)/rexfragment.o \
	$(OUTDIR)/rexnfasimulator.o \
	$(OUTDIR)/rexstate.o


INCLUDE = -I/usr/include/rpatk -I$(REX_SRCDIR) -I$(REX_SRCDIR)/..
LIBS = -lrpa -lrvm -lrlib -lpthread -lm
LDFLAGS += $(LIBS)
CFLAGS += $(INCLUDE) 

all: $(OUTDIR) $(REX_EXE)

$(OUTDIR)/%.o: $(REX_SRCDIR)/%.c
	$(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(REX_SRCDIR)/$*.c

$(OUTDIR)/%.o: $(REX_SRCDIR)/unix/%.c
	$(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(REX_SRCDIR)/unix/$*.c


$(OUTDIR)/%.o: $(REX_SRCDIR)/%.cpp
	$(CPP) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(REX_SRCDIR)/$*.cpp

$(OUTDIR)/%.o: $(REX_SRCDIR)/unix/%.cpp
	$(CPP) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(REX_SRCDIR)/unix/$*.cpp

$(REX_EXE): $(REX_OBJECTS)
	$(CPP) $(LDFLAGS) -o $@ $^

$(OUTDIR)/%.o: $(REX_RPADIR)/%.rpa
	$(OC) $(OCFLAGS_TXT)  $(REX_RPADIR)/$*.rpa $(OUTDIR)/$*.o


$(OUTDIR):
	@mkdir $(OUTDIR)

distclean: clean
	-rm -rf $(OUTDIR)

clean:
	-rm -f $(REX_EXE)
	-rm -f $(REX_OBJECTS)
	-rm -f *~
	-rm -f $(REX_SRCDIR)/*~

