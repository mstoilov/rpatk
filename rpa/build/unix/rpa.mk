RPA_SRCDIR = $(ROOT_DIR)/rpa
RPA_LIB = librpa.a
RPA_SO_VERSION = 2.0
RPA_SO_NAME = librpa.so
RPA_SO = $(RPA_SO_NAME).$(RPA_SO_VERSION)
TARGET_RPA_LIB = $(OUTDIR)/$(RPA_LIB)
TARGET_RPA_SO = $(OUTDIR)/$(RPA_SO)
CFLAGS += -I$(ROOT_DIR)

RPA_OBJECTS += $(OUTDIR)/rpacache.o
RPA_OBJECTS += $(OUTDIR)/rpadbex.o
RPA_OBJECTS += $(OUTDIR)/rpastat.o
RPA_OBJECTS += $(OUTDIR)/rparecord.o
RPA_OBJECTS += $(OUTDIR)/rpavm.o
RPA_OBJECTS += $(OUTDIR)/rpacompiler.o
RPA_OBJECTS += $(OUTDIR)/rpaparser.o
RPA_OBJECTS += $(OUTDIR)/rpaoptimization.o
RPA_OBJECTS += $(OUTDIR)/rpabitmap.o


ifeq ($(OS), linux)
all: $(OUTDIR) $(TARGET_RPA_LIB) $(TARGET_RPA_SO)
else
all: $(OUTDIR) $(TARGET_RPA_LIB)
endif


$(OUTDIR)/%.o: $(RPA_SRCDIR)/%.c
	$(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(RPA_SRCDIR)/$*.c

$(TARGET_RPA_LIB): $(RPA_OBJECTS)
	$(AR) -cr $@ $^

$(TARGET_RPA_SO): $(RPA_OBJECTS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(RPA_SO_NAME) -o $@ $^

$(OUTDIR):
	@mkdir $(OUTDIR)

distclean: clean
	-rm -f .depend
	-rm -rf $(OUTDIR)

clean:
	-rm -f $(TARGET_RPA_LIB)
	-rm -f $(TARGET_RPA_SO)
	-rm -f $(RPA_OBJECTS)
	-rm -f *~
	-rm -f $(ROOT_DIR)/*~

install:
	cp $(TARGET_RPA_SO) $(RPATK_LIB_INSTALL)
	cp $(TARGET_RPA_LIB) $(RPATK_LIB_INSTALL)
	cp $(RPA_SRCDIR)/*.h $(RPATK_INC_INSTALL)/rpa

uninstall:
	-rm -f $(RPATK_LIB_INSTALL)/$(RPA_LIB)
	-rm -f $(RPATK_LIB_INSTALL)/$(RPA_SO_NAME)*
	-rm -f $(RPATK_INC_INSTALL)/rpa/*
