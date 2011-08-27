OS		:= $(shell uname -s | tr A-Z a-z)
LOWCASEOS	:= $(shell echo $(OS) | tr A-Z a-z)


ifeq ($(ARCH), -m32)
ARCHDIR=i686
else
ARCHDIR = $(shell uname -m)
endif
OUTDIR = $(ARCHDIR)

ifeq ($(OS), Linux)
LIBS		= -static
endif

ROOTDIR		= ../..
RLIB_SRCDIR 	= $(ROOTDIR)/rlib
RVM_SRCDIR 	= $(ROOTDIR)/rvm
RPA_SRCDIR 	= $(ROOTDIR)/rpa
PUBDIR		= /home/build/releases/trunk
RPASDKDIR	= $(RPA_SRCDIR)
INCLUDES	= -I../ -I./ -I$(RPASDKDIR)
LIBS 		+= -L$(RLIB_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS 		+= -L$(RVM_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS 		+= -L$(RPA_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS		+= -lrpa -lrlib -lpthread
OBJECTS		= $(OUTDIR)/rpagrep.o $(OUTDIR)/main.o $(OUTDIR)/fsenum.o $(OUTDIR)/rpagrepdep.o $(OUTDIR)/rpagreputf.o
SRCS		= main.c fsenum.c rpagrep.c rpagrepdep.c rpagreputf.c
RPAGREP		= rgrep
RPASCGREP	= rscgrep
SRCDIR		= .
COMMONSRCDIR	= ..

include $(ROOTDIR)/build/unix/config.mk

ifeq ($(BLDCFG), release)
CFLAGS = -O3 -DUNIX $(INCLUDES)
else
CFLAGS = -ggdb -O0 -Wall -DUNIX -DDEBUG $(INCLUDES)
endif


all : $(OUTDIR) $(OUTDIR)/$(RPAGREP)

sc :  $(OUTDIR) $(OUTDIR)/$(RPASCGREP)

$(RPAGREP).tar.gz : $(OUTDIR) $(OUTDIR)/$(RPAGREP)
	mkdir -p $(RPAGREP)/$(OUTDIR)
	cp $(OUTDIR)/$(RPAGREP) $(RPAGREP)/$(OUTDIR)
	tar -czf $@ $(RPAGREP)
	rm -rf $(RPAGREP)

$(RPASCGREP).tar.gz : $(OUTDIR) $(OUTDIR)/$(RPASCGREP)
	mkdir -p $(RPASCGREP)/$(OUTDIR)
	cp $(OUTDIR)/$(RPASCGREP) $(RPASCGREP)/$(OUTDIR)
	tar -czf $@ $(RPASCGREP)
	rm -rf $(RPASCGREP)

publish : $(OUTDIR)
	make -C ../../librpa clean
	make -C ./ clean
	make -C ../../librpa sdk
	make -C ./ all
	make -C ./ $(RPAGREP).tar.gz
	scp $(RPAGREP).tar.gz build@www.rpasearch.com:$(PUBDIR)/$(OS)/$(OUTDIR)/$(RPAGREP)-$(OS)-$(OUTDIR).tar.gz

scpublish : $(OUTDIR)
	make -C ../../librpa clean
	make -C ./ clean
	make -C ../../librpa sdk
	make -C ./ all
	make -C ./ $(RPASCGREP).tar.gz
	scp $(RPASCGREP).tar.gz build@www.rpasearch.com:$(PUBDIR)/$(OS)/$(OUTDIR)/$(RPASCGREP)-$(OS)-$(OUTDIR).tar.gz


$(OUTDIR)/$(RPAGREP) : $(OBJECTS)
	$(CC) $(ARCH) -o $@ $^ $(LIBS) -lrpasx


$(OUTDIR)/$(RPASCGREP) : $(OBJECTS)
	$(CC) $(ARCH) -o $@ $^ $(LIBS) -lrpasc

$(OUTDIR)/%.o: $(COMMONSRCDIR)/%.c
	$(CC) $(ARCH) $(CFLAGS) -c -o $(OUTDIR)/$*.o $(COMMONSRCDIR)/$*.c

$(OUTDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(ARCH) $(CFLAGS) -c -o $(OUTDIR)/$*.o $(SRCDIR)/$*.c

$(OUTDIR) :
	@mkdir $(OUTDIR)


clean:
	@rm -rf *.o *~ $(OUTDIR)/$(RPAGREP) $(OBJECTS) $(RPAGREP).tar.gz $(RPASCGREP).tar.gz

distclean: clean
	@rm -rf i686 i386 x86_64 $(OUTDIR)



