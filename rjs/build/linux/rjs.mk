RLIB_SRCDIR = $(SRCDIR)/rlib
RVM_SRCDIR = $(SRCDIR)/rvm
RPA_SRCDIR = $(SRCDIR)/rpa
RJS_SRCDIR = $(SRCDIR)/rjs
RJS_LIB = $(OUTDIR)/librjs.a
RJS_SO = $(OUTDIR)/librjs.so.1.0
RJS_EXEC = $(OUTDIR)/rjsexec

CFLAGS += -I$(SRCDIR)

LIBS = -L$(RLIB_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RVM_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RPA_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RJS_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -lrjs -lrpa -lrvm -lrlib -lpthread -lm --static

RJS_OBJECTS =	\
	$(OUTDIR)/rjs.o \
	$(OUTDIR)/rjsparser.o \
	$(OUTDIR)/rjscompiler.o \
	$(OUTDIR)/rjsrules.o \
	$(OUTDIR)/rjsfile.o \
	$(OUTDIR)/ecma262.o \


RJSEXEC_OBJECTS =	\
	$(OUTDIR)/rjsexec.o \

ifeq ($(OS), linux)
all: $(OUTDIR) $(RJS_LIB) $(RJS_SO) $(RJS_EXEC)
else
all: $(OUTDIR) $(RJS_LIB)
endif

$(RJS_EXEC) : $(RJSEXEC_OBJECTS) $(RJS_LIB) $(RJS_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $< $(LIBS)

$(OUTDIR)/%.o: $(RJS_SRCDIR)/%.c
	$(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(RJS_SRCDIR)/$*.c

$(OUTDIR)/%.o: $(RJS_SRCDIR)/unix/%.c
	$(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(RJS_SRCDIR)/unix/$*.c

$(RJS_LIB): $(RJS_OBJECTS)
	$(AR) -cr $@ $^

$(RJS_SO): $(RJS_OBJECTS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,librjs.so -o $@ $^

$(OUTDIR)/%.o: $(RJS_SRCDIR)/%.rpa
	$(OC) --input binary --output $(ELFARCH) --binary-architecture $(BINARCH) $(RJS_SRCDIR)/$*.rpa $(OUTDIR)/$*.o

$(OUTDIR):
	@mkdir $(OUTDIR)

distclean: clean
	@rm -f .depend
	@rm -rf $(OUTDIR)

clean:
	@rm -f $(RJS_LIB)
	@rm -f $(RJS_SO)
	@rm -f $(RJS_OBJECTS)
	@rm -f $(RJS_EXEC)
	@rm -f *~
	@rm -f $(SRCDIR)/*~

