RAST_SRCDIR = $(SRCDIR)/rast
RAST_LIB = $(OUTDIR)/librast.a
RAST_SO = $(OUTDIR)/librast.so.1.0

RAST_OBJECTS =	\
	$(OUTDIR)/rastnode.o \


ifeq ($(OS), linux)
all: $(OUTDIR) $(RAST_LIB) $(RAST_SO)
else
all: $(OUTDIR) $(RAST_LIB)
endif


$(OUTDIR)/%.o: $(RAST_SRCDIR)/%.c
	$(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(RAST_SRCDIR)/$*.c

$(RAST_LIB): $(RAST_OBJECTS)
	$(AR) -cr $@ $^

$(RAST_SO): $(RAST_OBJECTS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,librast.so -o $@ $^

$(OUTDIR):
	@mkdir $(OUTDIR)

distclean: clean
	@rm -f .depend
	@rm -rf $(OUTDIR)

clean:
	@rm -f $(RAST_LIB)
	@rm -f $(RAST_SO)
	@rm -f $(RAST_OBJECTS)
	@rm -f *~
	@rm -f $(SRCDIR)/*~

