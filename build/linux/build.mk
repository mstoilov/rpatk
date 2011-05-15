all:
	+make -C $(SRCDIR)/rlib/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/rpa2/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/rvm/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/rjs/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/tests/testmisc/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/tests/testrpa2/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/tests/testrjs/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/rgrep/build/$(OS)/$(ARCHDIR) all

distclean: clean
	+make -C $(SRCDIR)/rlib/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/rpa2/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/rvm/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/rjs/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/tests/testmisc/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/tests/testrpa2/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/tests/testrjs/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/rgrep/build/$(OS)/$(ARCHDIR) distclean

clean:
	+make -C $(SRCDIR)/rlib/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/rpa2/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/rvm/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/rjs/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/tests/testmisc/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/tests/testrpa2/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/tests/testrjs/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/rgrep/build/$(OS)/$(ARCHDIR) clean

