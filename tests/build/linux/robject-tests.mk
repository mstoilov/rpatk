ROBJECT_SRCDIR = $(SRCDIR)/robject
RLIB_SRCDIR = $(SRCDIR)/rlib
RVM_SRCDIR = $(SRCDIR)/rvm
RPA_SRCDIR = $(SRCDIR)/rpa
RAST_SRCDIR = $(SRCDIR)/rast
TESTS_SRCDIR = $(SRCDIR)/tests
INCLUDE = -I$(SRCDIR)/arch/$(OS)/$(ARCHDIR) -I$(ROBJECT_SRCDIR) -I$(RLIB_SRCDIR) -I$(RVM_SRCDIR) -I$(RPA_SRCDIR) -I$(RAST_SRCDIR)
LIBS =  -L$(ROBJECT_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RLIB_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RVM_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RPA_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RAST_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -lrvm -lrpa -lrpasx -lrast -lrlib -lpthread -lm --static


TESTS	+= $(OUTDIR)/rast-test
TESTS	+= $(OUTDIR)/rpagen-test
TESTS	+= $(OUTDIR)/calc-test
TESTS	+= $(OUTDIR)/funcarg-test
TESTS   += $(OUTDIR)/codegen-test
TESTS   += $(OUTDIR)/codemap-test
TESTS   += $(OUTDIR)/opmap-test
TESTS   += $(OUTDIR)/string-test
TESTS   += $(OUTDIR)/rlock-test
TESTS   += $(OUTDIR)/rarray-test
TESTS   += $(OUTDIR)/rcarray-test
TESTS   += $(OUTDIR)/rharray-test
TESTS   += $(OUTDIR)/scope-test
TESTS   += $(OUTDIR)/rhash-test
TESTS   += $(OUTDIR)/rvm-test
TESTS   += $(OUTDIR)/loop-test
TESTS   += $(OUTDIR)/speed-test
TESTS   += $(OUTDIR)/memalloc-test
TESTS   += $(OUTDIR)/asm-cast
TESTS   += $(OUTDIR)/asm-add
TESTS   += $(OUTDIR)/asm-adds
TESTS   += $(OUTDIR)/asm-b
TESTS   += $(OUTDIR)/asm-bitops
TESTS   += $(OUTDIR)/asm-callback
TESTS   += $(OUTDIR)/asm-clz
TESTS   += $(OUTDIR)/asm-cmp
TESTS   += $(OUTDIR)/asm-div
TESTS   += $(OUTDIR)/asm-loadstore
TESTS   += $(OUTDIR)/asm-mul
TESTS   += $(OUTDIR)/asm-sbc
TESTS   += $(OUTDIR)/asm-shiftops
TESTS   += $(OUTDIR)/asm-stack
TESTS   += $(OUTDIR)/asm-bl
TESTS   += $(OUTDIR)/asm-ecmp
TESTS   += $(OUTDIR)/asm-esub
TESTS   += $(OUTDIR)/asm-eadd


all : $(OUTDIR) $(TESTS)

$(OUTDIR)/%.o: $(TESTS_SRCDIR)/%.c
	+ $(CC) $(CFLAGS) -c -o $(OUTDIR)/$*.o $(TESTS_SRCDIR)/$*.c $(INCLUDE)

$(OUTDIR)/rpagen-test : $(OUTDIR)/ecma262.o $(OUTDIR)/rpagen-test.o
	$(CC) $(CFLAGS)  -o $@ $^ $(LIBS)

$(OUTDIR)/rast-test : $(OUTDIR)/ecma262.o $(OUTDIR)/rast-test.o
	$(CC) $(CFLAGS)  -o $@ $^ $(LIBS)

$(OUTDIR)/%: $(TESTS_SRCDIR)/%.c
	+ $(CC) $(CFLAGS) -o $(OUTDIR)/$* $(TESTS_SRCDIR)/$*.c $(LIBS) $(INCLUDE)


$(OUTDIR)/%.o: $(TESTS_SRCDIR)/%.rpa
	$(LD) -r -b binary -o $(OUTDIR)/$*.o $(TESTS_SRCDIR)/$*.rpa


$(OUTDIR):
	@mkdir $(OUTDIR)

distclean: clean
	@rm -f .depend
	@rm -rf $(OUTDIR)

clean:
	@rm -f $(TESTS)
	@rm -f *~
	@rm -f $(SRCDIR)/*~

