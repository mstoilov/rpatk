TEMPLATE_SRCDIR = $(ROOT_DIR)/robject
TEMPLATE_LIB = $(OUTDIR)/librobject.a
TEMPLATE_SO = $(OUTDIR)/librobject.so.1.0

TEMPLATE_OBJECTS =	\
	$(OUTDIR)/robject.o \


ifeq ($(OS), linux)
all: $(OUTDIR) $(TEMPLATE_LIB) $(TEMPLATE_SO)
else
all: $(OUTDIR) $(TEMPLATE_LIB)
endif


$(OUTDIR)/%.o: $(TEMPLATE_SRCDIR)/%.c
	$(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(TEMPLATE_SRCDIR)/$*.c

$(TEMPLATE_LIB): $(TEMPLATE_OBJECTS)
	$(AR) -cr $@ $^

$(TEMPLATE_SO): $(TEMPLATE_OBJECTS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,librobject.so -o $@ $^

$(OUTDIR):
	@mkdir $(OUTDIR)

distclean: clean
	-rm -f .depend
	-rm -rf $(OUTDIR)

clean:
	-rm -f $(TEMPLATE_LIB)
	-rm -f $(TEMPLATE_SO)
	-rm -f $(TEMPLATE_OBJECTS)
	-rm -f *~
	-rm -f $(ROOT_DIR)/*~

