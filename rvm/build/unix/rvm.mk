RVM_SRCDIR = $(ROOT_DIR)/rvm
RVM_SO_VERSION = 2.0
RVM_SO_NAME = librvm.so
RVM_LIB = librvm.a
RVM_SO = $(RVM_SO_NAME).$(RVM_SO_VERSION)
TARGET_RVM_LIB = $(OUTDIR)/$(RVM_LIB)
TARGET_RVM_SO = $(OUTDIR)/$(RVM_SO)
CFLAGS += -I$(ROOT_DIR)

RVM_OBJECTS +=	$(OUTDIR)/rvmcpu.o 
RVM_OBJECTS +=	$(OUTDIR)/rvmcodemap.o
RVM_OBJECTS +=	$(OUTDIR)/rvmrelocmap.o
RVM_OBJECTS +=	$(OUTDIR)/rvmcodegen.o
RVM_OBJECTS +=	$(OUTDIR)/rvmreg.o
RVM_OBJECTS +=	$(OUTDIR)/rvmscope.o


ifeq ($(OS), linux)
all: $(OUTDIR) $(TARGET_RVM_LIB) $(TARGET_RVM_SO)
else
all: $(OUTDIR) $(TARGET_RVM_LIB)
endif

$(OUTDIR)/%.o: $(RVM_SRCDIR)/%.c
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(RVM_SRCDIR)/$*.c

$(TARGET_RVM_LIB): $(RVM_OBJECTS)
	$(AR) -cr $@ $^

$(TARGET_RVM_SO): $(RVM_OBJECTS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(RVM_SO_NAME) -o $@ $^

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
	-rm -f $(ROOT_DIR)/*~

install:
	cp $(TARGET_RVM_SO) $(RPATK_LIB_INSTALL)
	cp $(TARGET_RVM_LIB) $(RPATK_LIB_INSTALL)
	cp $(RVM_SRCDIR)/*.h $(RPATK_INC_INSTALL)/rvm

uninstall:
	-rm -f $(RPATK_LIB_INSTALL)/$(RVM_LIB)
	-rm -f $(RPATK_LIB_INSTALL)/$(RVM_SO_NAME)*
	-rm -f $(RPATK_INC_INSTALL)/rvm/*
