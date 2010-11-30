RPA_SRCDIR = $(SRCDIR)/rpa


RPA_LIBNAME = librpa
RPASX_LIBNAME = librpasx
RPA_LIB = $(OUTDIR)/$(RPA_LIBNAME).a
RPA_SO = $(OUTDIR)/$(RPA_LIBNAME).so.1.0
RPASX_LIB = $(OUTDIR)/$(RPASX_LIBNAME).a
RPASX_SO = $(OUTDIR)/$(RPASX_LIBNAME).so.1.0

CFLAGS += -I$(RPA_SRCDIR)/config -I$(SRCDIR)/rlib
CFLAGS += -DRPA_DEBUG_MEM 
CFLAGS += -DHAVESTDIO
#CFLAGS += -DDEBUGPRINT
#CFLAGS += -DCODEGENDEBUG
#CFLAGS += -DVMEXECDEBUG 
#CFLAGS += -DRPANOSTACKCHEK




RPA_OBJECTS +=	$(OUTDIR)/rpaclass.o
RPA_OBJECTS +=	$(OUTDIR)/rpautf.o
RPA_OBJECTS +=	$(OUTDIR)/rpacharconv.o
RPA_OBJECTS +=	$(OUTDIR)/rpamnode.o
RPA_OBJECTS +=	$(OUTDIR)/rpamatch.o
RPA_OBJECTS +=	$(OUTDIR)/rpamatchspecial.o
RPA_OBJECTS +=	$(OUTDIR)/rpamatchrangelist.o
RPA_OBJECTS +=	$(OUTDIR)/rpamatchval.o
RPA_OBJECTS +=	$(OUTDIR)/rpamatchlist.o
RPA_OBJECTS +=	$(OUTDIR)/rpamatchstr.o
RPA_OBJECTS +=	$(OUTDIR)/rpamatchrange.o
RPA_OBJECTS +=	$(OUTDIR)/rpastat.o
RPA_OBJECTS +=	$(OUTDIR)/rpavar.o
RPA_OBJECTS +=	$(OUTDIR)/rpavarlink.o
RPA_OBJECTS +=	$(OUTDIR)/rpadebug.o
RPA_OBJECTS +=	$(OUTDIR)/rpaparser.o
RPA_OBJECTS +=	$(OUTDIR)/rpavm.o
RPA_OBJECTS +=	$(OUTDIR)/rpawordstack.o
RPA_OBJECTS +=	$(OUTDIR)/rpaposmnodestack.o
RPA_OBJECTS +=	$(OUTDIR)/rpadbex.o
RPA_OBJECTS +=	$(OUTDIR)/rpadbexpriv.o
RPA_OBJECTS +=	$(OUTDIR)/rpacbrecord.o


RPASX_OBJECTS += $(OUTDIR)/rpasearch.o 



ifeq ($(OS), linux)
all: $(OUTDIR) $(RPA_LIB) $(RPA_SO) $(RPASX_LIB) $(RPASX_SO)
else
all: $(OUTDIR) $(RPA_LIB) $(RPASX_LIB)
endif


$(OUTDIR)/%.o: $(RPA_SRCDIR)/%.c
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(RPA_SRCDIR)/$*.c

$(RPA_LIB): $(RPA_OBJECTS)
	$(AR) -cr $@ $^

$(RPA_SO): $(RPA_OBJECTS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,librpa.so -o $@ $^

$(RPASX_LIB): $(RPASX_OBJECTS)
	$(AR) -cr $@ $^

$(RPASX_SO): $(RPASX_OBJECTS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(RPASX_LIBNAME).so -o $(RPASX_SO) $^


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

