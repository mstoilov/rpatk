RPA_SRCDIR = $(SRCDIR)/rpa2
RPA_LIB = $(OUTDIR)/librpa2.a
RPA_SO = $(OUTDIR)/librpa2.so.1.0

CFLAGS += -I$(RVM_SRCDIR)/config -I$(SRCDIR)/rlib -I$(SRCDIR)/rvm

RPA_OBJECTS =	\
	$(OUTDIR)/rpastat.o \
	$(OUTDIR)/rpavm.o \
	$(OUTDIR)/rpacompiler.o \


ifeq ($(OS), linux)
all: $(OUTDIR) $(RPA_LIB) $(RPA_SO)
else
all: $(OUTDIR) $(RPA_LIB)
endif


$(OUTDIR)/%.o: $(RPA_SRCDIR)/%.c
	$(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(RPA_SRCDIR)/$*.c

$(RPA_LIB): $(RPA_OBJECTS)
	$(AR) -cr $@ $^

$(RPA_SO): $(RPA_OBJECTS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,librpa2.so -o $@ $^

$(OUTDIR):
	@mkdir $(OUTDIR)

distclean: clean
	@rm -f .depend
	@rm -rf $(OUTDIR)

clean:
	@rm -f $(RPA_LIB)
	@rm -f $(RPA_SO)
	@rm -f $(RPA_OBJECTS)
	@rm -f *~
	@rm -f $(SRCDIR)/*~

