RTK_LIB_INSTALL=/usr/lib
RVM_SRCDIR = $(SRCDIR)/rvm
RVM_SO_VERSION = 2.0
RVM_SO_NAME = librvm.so
RVM_LIB = librvm.a
RVM_SO = $(RVM_SO_NAME).$(RVM_SO_VERSION)
TARGET_RVM_LIB = $(OUTDIR)/$(RVM_LIB)
TARGET_RVM_SO = $(OUTDIR)/$(RVM_SO)

CFLAGS += -I$(SRCDIR)

RVM_OBJECTS +=	$(OUTDIR)/rvmcpu.o 
RVM_OBJECTS +=	$(OUTDIR)/rvmoperator.o
RVM_OBJECTS +=	$(OUTDIR)/rvmcodemap.o
RVM_OBJECTS +=	$(OUTDIR)/rvmrelocmap.o
RVM_OBJECTS +=	$(OUTDIR)/rvmcodegen.o
RVM_OBJECTS +=	$(OUTDIR)/rvmreg.o
RVM_OBJECTS +=	$(OUTDIR)/rvmscope.o
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


all: $(OUTDIR) $(TARGET_RVM_LIB)


$(OUTDIR)/%.o: $(RVM_SRCDIR)/%.c
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(RVM_SRCDIR)/$*.c

$(TARGET_RVM_LIB): $(RVM_OBJECTS)
	$(AR) -cr $@ $^

$(OUTDIR):
	@mkdir $(OUTDIR)

distclean: clean
	-rm -f .depend
	-rm -rf $(OUTDIR)

clean:
	-rm -f $(TARGET_RVM_LIB)
	-rm -f $(TARGET_RVM_SO)
	-rm -f $(RVM_OBJECTS)
	-rm -f *~
	-rm -f $(SRCDIR)/*~

install:
	cp $(TARGET_RVM_LIB) $(RTK_LIB_INSTALL)
	cp $(RVM_SRCDIR)/*.h $(RPATK_INC_INSTALL)/rvm

uninstall:
	-rm -f $(RTK_LIB_INSTALL)/$(RVM_LIB)
	-rm -f $(RPATK_INC_INSTALL)/rvm/*
