ROBJECT_SRCDIR = $(SRCDIR)/robject
ROBJECT_LIB = $(OUTDIR)/librobject.a
ROBJECT_SO = $(OUTDIR)/librobject.so.1.0

ROBJECT_OBJECTS =	\
	$(OUTDIR)/robject.o \


ifeq ($(OS), linux)
all: $(OUTDIR) $(ROBJECT_LIB) $(ROBJECT_SO)
else
all: $(OUTDIR) $(ROBJECT_LIB)
endif


$(OUTDIR)/%.o: $(ROBJECT_SRCDIR)/%.c
	$(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(ROBJECT_SRCDIR)/$*.c

$(ROBJECT_LIB): $(ROBJECT_OBJECTS)
	$(AR) -cr $@ $^

$(ROBJECT_SO): $(ROBJECT_OBJECTS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,librobject.so -o $@ $^

$(OUTDIR):
	@mkdir $(OUTDIR)

distclean: clean
	@rm -f .depend
	@rm -rf $(OUTDIR)

clean:
	@rm -f $(ROBJECT_LIB)
	@rm -f $(ROBJECT_SO)
	@rm -f $(ROBJECT_OBJECTS)
	@rm -f *~
	@rm -f $(SRCDIR)/*~

