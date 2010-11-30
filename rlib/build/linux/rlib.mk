RLIB_SRCDIR = $(SRCDIR)/rlib
RLIB_LIB = $(OUTDIR)/librlib.a
RLIB_SO = $(OUTDIR)/librlib.so.1.0


RLIB_OBJECTS +=	$(OUTDIR)/rref.o
RLIB_OBJECTS +=	$(OUTDIR)/robject.o
RLIB_OBJECTS +=	$(OUTDIR)/rmem.o
RLIB_OBJECTS +=	$(OUTDIR)/ratomic.o
RLIB_OBJECTS +=	$(OUTDIR)/rspinlock.o
RLIB_OBJECTS +=	$(OUTDIR)/rharray.o
RLIB_OBJECTS +=	$(OUTDIR)/rarray.o
RLIB_OBJECTS +=	$(OUTDIR)/rhash.o
RLIB_OBJECTS +=	$(OUTDIR)/rstring.o
RLIB_OBJECTS +=	$(OUTDIR)/rlist.o


ifeq ($(OS), linux)
all: $(OUTDIR) $(RLIB_LIB) $(RLIB_SO)
else
all: $(OUTDIR) $(RLIB_LIB)
endif


$(OUTDIR)/%.o: $(RLIB_SRCDIR)/%.c
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(RLIB_SRCDIR)/$*.c

$(RLIB_LIB): $(RLIB_OBJECTS)
	$(AR) -cr $@ $^

$(RLIB_SO): $(RLIB_OBJECTS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,librlib.so -o $@ $^

$(OUTDIR):
	@mkdir $(OUTDIR)

distclean: clean
	@rm -f .depend
	@rm -rf $(OUTDIR)

clean:
	@rm -f $(RLIB_LIB)
	@rm -f $(RLIB_SO)
	@rm -f $(RLIB_OBJECTS)
	@rm -f *~
	@rm -f $(SRCDIR)/*~

