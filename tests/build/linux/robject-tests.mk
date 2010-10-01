ROBJECT_SRCDIR = $(SRCDIR)/robject
RLIB_SRCDIR = $(SRCDIR)/rlib
RVM_SRCDIR = $(SRCDIR)/rvm
TESTS_SRCDIR = $(SRCDIR)/tests
INCLUDE = -I$(SRCDIR)/arch/$(OS)/$(ARCHDIR) -I$(ROBJECT_SRCDIR) -I$(RLIB_SRCDIR) -I$(RVM_SRCDIR)
LIBS =  -L$(ROBJECT_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RLIB_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -L$(RVM_SRCDIR)/build/$(OS)/$(ARCHDIR)/out 
LIBS += -lrvm -lrlib -lpthread --static


TESTS	= \
	$(OUTDIR)/codegen-test \
	$(OUTDIR)/codemap-test \
	$(OUTDIR)/opmap-test \
	$(OUTDIR)/string-test \
	$(OUTDIR)/rlock-test \
	$(OUTDIR)/rarray-test \
	$(OUTDIR)/scope-test \
	$(OUTDIR)/rhash-test \
	$(OUTDIR)/rvm-test \
	$(OUTDIR)/memalloc-test \
	$(OUTDIR)/asm-add \
	$(OUTDIR)/asm-adds \
	$(OUTDIR)/asm-b \
	$(OUTDIR)/asm-bitops \
	$(OUTDIR)/asm-callback \
	$(OUTDIR)/asm-clz \
	$(OUTDIR)/asm-cmp \
	$(OUTDIR)/asm-div \
	$(OUTDIR)/asm-loadstore \
	$(OUTDIR)/asm-mul \
	$(OUTDIR)/asm-sbc \
	$(OUTDIR)/asm-shiftops \
	$(OUTDIR)/asm-stack \
	$(OUTDIR)/asm-bl	


all : $(OUTDIR) $(TESTS)

$(OUTDIR)/%: $(TESTS_SRCDIR)/%.c
	$(CC) $(CFLAGS) -o $(OUTDIR)/$* $(TESTS_SRCDIR)/$*.c $(LIBS) $(INCLUDE)


$(OUTDIR):
	@mkdir $(OUTDIR)

distclean: clean
	@rm -f .depend
	@rm -rf $(OUTDIR)

clean:
	@rm -f $(TESTS)
	@rm -f *~
	@rm -f $(SRCDIR)/*~

