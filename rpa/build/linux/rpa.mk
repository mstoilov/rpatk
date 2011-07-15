RPA_SRCDIR = $(SRCDIR)/rpa
RPA_LIB = librpa.a
RPA_SO_VERSION = 2.0
RPA_SO_NAME = librpa.so
RPA_SO = $(RPA_SO_NAME).$(RPA_SO_VERSION)
TARGET_RPA_LIB = $(OUTDIR)/$(RPA_LIB)
TARGET_RPA_SO = $(OUTDIR)/$(RPA_SO)


CFLAGS += -I$(SRCDIR)

RPA_OBJECTS =	\
	$(OUTDIR)/rpacache.o \
	$(OUTDIR)/rpadbex.o \
	$(OUTDIR)/rpastat.o \
	$(OUTDIR)/rparecord.o \
	$(OUTDIR)/rpavm.o \
	$(OUTDIR)/rpacompiler.o \
	$(OUTDIR)/rpaparser.o \
	$(OUTDIR)/rpaoptimization.o \
	$(OUTDIR)/rpabitmap.o \


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
	@rm -f .depend
	@rm -rf $(OUTDIR)

clean:
	@rm -f $(TARGET_RPA_LIB)
	@rm -f $(TARGET_RPA_SO)
	@rm -f $(RPA_OBJECTS)
	@rm -f *~
	@rm -f $(SRCDIR)/*~

install:
	cp $(TARGET_RPA_SO) $(RTK_LIB_INSTALL)
	cp $(TARGET_RPA_LIB) $(RTK_LIB_INSTALL)
	cp $(RPA_SRCDIR)/*.h $(RPATK_INC_INSTALL)/rpa

uninstall:
	rm $(RTK_LIB_INSTALL)/$(RPA_LIB)
	rm $(RTK_LIB_INSTALL)/$(RPA_SO)
	rm $(RPATK_INC_INSTALL)/rpa/*
