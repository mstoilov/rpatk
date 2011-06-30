RPATK_LIB_INSTALL=/usr/lib
RLIB_SRCDIR = $(SRCDIR)/rlib
RLIB_SO_VERSION = 2.0
RLIB_SO_NAME = librlib.so
RLIB_LIB = librlib.a
RLIB_SO = $(RLIB_SO_NAME).$(RLIB_SO_VERSION)
RLIB_LIB = librlib.a
TARGET_RLIB_LIB = $(OUTDIR)/$(RLIB_LIB)
TARGET_RLIB_SO = $(OUTDIR)/$(RLIB_SO)

CFLAGS += -I$(SRCDIR)

RLIB_OBJECTS +=	$(OUTDIR)/rref.o
RLIB_OBJECTS +=	$(OUTDIR)/rcharconv.o
RLIB_OBJECTS +=	$(OUTDIR)/robject.o
RLIB_OBJECTS +=	$(OUTDIR)/rgc.o
RLIB_OBJECTS +=	$(OUTDIR)/rmem.o
RLIB_OBJECTS +=	$(OUTDIR)/rmath.o
RLIB_OBJECTS +=	$(OUTDIR)/ratomic.o
RLIB_OBJECTS +=	$(OUTDIR)/rspinlock.o
RLIB_OBJECTS +=	$(OUTDIR)/rharray.o
RLIB_OBJECTS +=	$(OUTDIR)/rcarray.o
RLIB_OBJECTS +=	$(OUTDIR)/rarray.o
RLIB_OBJECTS +=	$(OUTDIR)/rhash.o
RLIB_OBJECTS +=	$(OUTDIR)/rmap.o
RLIB_OBJECTS +=	$(OUTDIR)/rstring.o
RLIB_OBJECTS +=	$(OUTDIR)/rlist.o
RLIB_OBJECTS +=	$(OUTDIR)/rutf.o


ifeq ($(OS), linux)
all: $(OUTDIR) $(TARGET_RLIB_LIB) $(TARGET_RLIB_SO)
else
all: $(OUTDIR) $(TARGET_RLIB_LIB)
endif


$(OUTDIR)/%.o: $(RLIB_SRCDIR)/%.c
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(RLIB_SRCDIR)/$*.c

$(TARGET_RLIB_LIB): $(RLIB_OBJECTS)
	$(AR) -cr $@ $^

$(TARGET_RLIB_SO): $(RLIB_OBJECTS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(RLIB_SO_NAME) -o $@ $^

$(OUTDIR):
	@mkdir $(OUTDIR)

distclean: clean
	@rm -f .depend
	@rm -rf $(OUTDIR)

clean:
	@rm -f $(TARGET_RLIB_LIB)
	@rm -f $(TARGET_RLIB_SO)
	@rm -f $(RLIB_OBJECTS)
	@rm -f *~
	@rm -f $(SRCDIR)/*~

install:
	cp $(TARGET_RLIB_SO) $(RPATK_LIB_INSTALL)
	cp $(TARGET_RLIB_LIB) $(RPATK_LIB_INSTALL)
	cp $(RLIB_SRCDIR)/*.h $(RPATK_INC_INSTALL)/rlib


uninstall:
	rm $(RPATK_LIB_INSTALL)/$(RLIB_LIB)
	rm $(RPATK_LIB_INSTALL)/$(RLIB_SO)
	rm $(RPATK_INC_INSTALL)/rlib/*
