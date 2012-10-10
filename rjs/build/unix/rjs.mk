RLIB_SRCDIR = $(ROOT_DIR)/rlib
RVM_SRCDIR = $(ROOT_DIR)/rvm
RPA_SRCDIR = $(ROOT_DIR)/rpa
RJS_SRCDIR = $(ROOT_DIR)/rjs
RJS_LIB = $(OUTDIR)/librjs.a
RJS_SO = $(OUTDIR)/librjs.so.1.0
RJS_EXEC = $(OUTDIR)/rjsexec
LDFLAGS_RJSEXEC = $(LDFLAGS)
ifeq ($(OS), linux)
LDFLAGS_RJSEXEC += --static
endif

ifeq ($(OS), darwin)
ECMA262_SECTION = -sectcreate rpa ecma262 $(RJS_SRCDIR)/ecma262.rpa
endif

CFLAGS += -I$(ROOT_DIR)

LIBS = -L$(RLIB_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(RVM_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(RPA_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -L$(RJS_SRCDIR)/build/unix/$(ARCHDIR)/out 
LIBS += -lrjs -lrpa -lrvm -lrlib -lpthread -lm

RJS_OBJECTS += $(OUTDIR)/rjs.o
RJS_OBJECTS += $(OUTDIR)/rjsparser.o
RJS_OBJECTS += $(OUTDIR)/rjscompiler.o
RJS_OBJECTS += $(OUTDIR)/rjsrules.o
RJS_OBJECTS += $(OUTDIR)/rjsfile.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperator.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatorbin.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatoradd.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatorand.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatoreq.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatornoteq.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatorlogicor.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatorlogicand.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatorlogicnot.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatorless.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatorlesseq.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatorgreater.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatorgreatereq.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatorxor.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatoror.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatorcmp.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatorcmn.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatorlsl.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatorlsr.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatorlsru.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatorcast.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatorcat.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatorsub.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatormul.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatordiv.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatormod.o
RJS_OBJECTS +=	$(OUTDIR)/rvmoperatornot.o


ifeq ($(OS), linux)
RJS_OBJECTS += $(OUTDIR)/ecma262.o
endif

RJSEXEC_OBJECTS =	\
	$(OUTDIR)/rjsexec.o \



ifeq ($(OS), linux)
all: $(OUTDIR) $(RJS_LIB) $(RJS_SO) $(RJS_EXEC)
else
all: $(OUTDIR) $(RJS_LIB) $(RJS_EXEC)
endif

$(RJS_EXEC) : $(RJSEXEC_OBJECTS) $(RJS_LIB) $(RJS_OBJECTS)
	$(CC) -o $@ $< $(LIBS) $(LDFLAGS_RJSEXEC) $(ECMA262_SECTION)

$(OUTDIR)/%.o: $(RJS_SRCDIR)/%.c
	$(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(RJS_SRCDIR)/$*.c

ifeq ($(OS), darwin)
$(OUTDIR)/%.o: $(RJS_SRCDIR)/darwin/%.c
	$(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(RJS_SRCDIR)/darwin/$*.c
endif

$(OUTDIR)/%.o: $(RJS_SRCDIR)/unix/%.c
	$(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(RJS_SRCDIR)/unix/$*.c

$(RJS_LIB): $(RJS_OBJECTS)
	$(AR) -cr $@ $^

$(RJS_SO): $(RJS_OBJECTS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,librjs.so -o $@ $^

$(OUTDIR)/%.o: $(RJS_SRCDIR)/%.rpa
	$(OBJCOPY) $(OCFLAGS_TXT)  $(RJS_SRCDIR)/$*.rpa $(OUTDIR)/$*.o

$(OUTDIR):
	@mkdir $(OUTDIR)

distclean: clean
	-rm -f .depend
	-rm -rf $(OUTDIR)

clean:
	-rm -f $(RJS_LIB)
	-rm -f $(RJS_SO)
	-rm -f $(RJS_OBJECTS)
	-rm -f $(RJS_EXEC)
	-rm -f *~
	-rm -f $(ROOT_DIR)/*~

