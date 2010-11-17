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



RPA_OBJECTS = \
	$(OUTDIR)/rpaclass.o \
	$(OUTDIR)/rpautf.o \
	$(OUTDIR)/rpacharconv.o \
	$(OUTDIR)/rpamnode.o \
	$(OUTDIR)/rpamatch.o \
	$(OUTDIR)/rpamatchspecial.o \
	$(OUTDIR)/rpamatchrangelist.o \
	$(OUTDIR)/rpamatchval.o \
	$(OUTDIR)/rpamatchlist.o \
	$(OUTDIR)/rpamatchstr.o \
	$(OUTDIR)/rpamatchrange.o \
	$(OUTDIR)/rpastat.o \
	$(OUTDIR)/rpavar.o \
	$(OUTDIR)/rpavarlink.o \
	$(OUTDIR)/rpadebug.o \
	$(OUTDIR)/rpaparser.o \
	$(OUTDIR)/rpavm.o \
	$(OUTDIR)/rpawordstack.o \
	$(OUTDIR)/rpaposmnodestack.o \
	$(OUTDIR)/rpadbex.o \
	$(OUTDIR)/rpadbexpriv.o \
	$(OUTDIR)/rpacbrecord.o

RPASX_OBJECTS =	\
	$(OUTDIR)/rpasearch.o \



ifeq ($(OS), linux)
all: $(OUTDIR) $(RPA_LIB) $(RPA_SO) $(RPASX_LIB) $(RPASX_SO)
else
all: $(OUTDIR) $(RPA_LIB) $(RPASX_LIB)
endif


$(OUTDIR)/%.o: $(RPA_SRCDIR)/%.c
	$(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(RPA_SRCDIR)/$*.c

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

