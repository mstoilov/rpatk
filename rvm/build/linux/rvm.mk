RVM_SRCDIR = $(SRCDIR)/rvm
RVM_LIB = $(OUTDIR)/librvm.a
RVM_SO = $(OUTDIR)/librvm.so.1.0

CFLAGS += -I$(RVM_SRCDIR)/config -I$(SRCDIR)/rlib


RVM_OBJECTS +=	$(OUTDIR)/rvmcpu.o 
RVM_OBJECTS +=	$(OUTDIR)/rvmoperator.o
RVM_OBJECTS +=	$(OUTDIR)/rvmcodemap.o
RVM_OBJECTS +=	$(OUTDIR)/rvmrelocmap.o
RVM_OBJECTS +=	$(OUTDIR)/rvmcodegen.o
RVM_OBJECTS +=	$(OUTDIR)/rvmreg.o
RVM_OBJECTS +=	$(OUTDIR)/rvmscope.o
RVM_OBJECTS +=	$(OUTDIR)/rvmgc.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatorbin.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatoradd.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatorand.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatoreq.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatornoteq.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatorlogicor.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatorlogicand.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatorlogicnot.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatorless.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatorlesseq.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatorgreater.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatorgreatereq.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatorxor.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatoror.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatorcmp.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatorcmn.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatorlsl.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatorlsr.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatorlsru.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatorcast.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatorcat.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatorsub.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatormul.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatordiv.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatormod.o
RVM_OBJECTS +=	$(OUTDIR)/rvmoperatornot.o


ifeq ($(OS), linux)
all: $(OUTDIR) $(RVM_LIB) $(RVM_SO)
else
all: $(OUTDIR) $(RVM_LIB)
endif


$(OUTDIR)/%.o: $(RVM_SRCDIR)/%.c
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(RVM_SRCDIR)/$*.c

$(RVM_LIB): $(RVM_OBJECTS)
	$(AR) -cr $@ $^

$(RVM_SO): $(RVM_OBJECTS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,librvm.so -o $@ $^

$(OUTDIR):
	@mkdir $(OUTDIR)

distclean: clean
	@rm -f .depend
	@rm -rf $(OUTDIR)

clean:
	@rm -f $(RVM_LIB)
	@rm -f $(RVM_SO)
	@rm -f $(RVM_OBJECTS)
	@rm -f *~
	@rm -f $(SRCDIR)/*~

