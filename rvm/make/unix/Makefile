SRCDIR = ../..
ifeq ($(ARCH), -m32)
OUTDIR=i686
else
OUTDIR = $(shell uname -m)
endif
INCLUDE = -I$(SRCDIR)/include -I$(SRCDIR)/config -I$(SRCDIR)
OS = $(shell uname)
RPASDKDIR = ../../../../../../svn.rpa/rpa/trunk/rpasdk/$(OS)/$(OUTDIR)
RPAINSTDIR = /usr/lib
RPAINSTHEADERDIR = /usr/include/rpasdk

CC = gcc
AR = ar
ifeq ($(BLDCFG), release)
CFLAGS = -O3 -fPIC 
else
CFLAGS = -fPIC -ggdb -O0 -Wall
endif

ifeq ($(CCBLD), yes)
CFLAGS += -fprofile-arcs -ftest-coverage
endif

CFLAGS += $(ARCH) $(INCLUDE) 
# CFLAGS += -RPA_LONGLONGINT
CFLAGS += -DDEBUG 
CFLAGS += -DRPA_DEBUG_MEM 
CFLAGS += -DHAVESTDIO

CFLAGS := $(CFLAGS)
LDFLAGS = $(ARCH)
REGVM_LIBNAME = libregvm
REGVM_LIB = $(OUTDIR)/$(REGVM_LIBNAME).a
REGVM_SO = $(OUTDIR)/$(REGVM_LIBNAME).so.1.0


REGVM_OBJECTS = \
	$(OUTDIR)/rvm.o \


ifeq ($(OS), Linux)
all: $(OUTDIR) $(REGVM_LIB) $(REGVM_SO)
else
all: $(OUTDIR) $(REGVM_LIB)
endif

$(OUTDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -o $(OUTDIR)/$*.o -c $(SRCDIR)/$*.c

$(REGVM_LIB): $(REGVM_OBJECTS)
	$(AR) -cr $@ $^


$(REGVM_SO): $(REGVM_OBJECTS)
	$(CC) -shared $(LDFLAGS) -Wl,-soname,$(REGVM_LIBNAME).so -o $(REGVM_SO) $^

$(OUTDIR):
	@mkdir $(OUTDIR)

distclean: clean
	@rm -f .depend
	@rm -rf $(OUTDIR)

clean:
	@rm -f $(REGVM_LIB)
	@rm -f $(REGVM_SO)
	@rm -f $(REGVM_OBJECTS)
	@rm -f *~
	@rm -f $(SRCDIR)/*~

$(RPASDKDIR) : 
	@mkdir -p $(RPASDKDIR)

sdk: all $(RPASDKDIR)
	cp $(SRCDIR)/rvm.h $(RPASDKDIR)
ifeq ($(OS), Linux)
	cp $(REGVM_SO) $(RPASDKDIR)
endif
	cp $(REGVM_LIB) $(RPASDKDIR)



$(RPAINSTHEADERDIR) :
	@mkdir $(RPAINSTHEADERDIR)

install: all $(RPAINSTDIR) $(RPAINSTHEADERDIR)
ifeq ($(OS), Linux)
	cp $(REGVM_SO) $(RPAINSTDIR)
endif
	cp $(REGVM_LIB) $(RPAINSTDIR)


uninstall:
	-rm $(RPAINSTDIR)/$(REGVM_LIBNAME).*
	-rm -rf $(RPAINSTHEADERDIR)
