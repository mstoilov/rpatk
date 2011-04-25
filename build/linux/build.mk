all:
	+make -C $(SRCDIR)/rlib/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/rast/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/rpa1/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/rpa2/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/rvm/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/rjs/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/tests/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/testrpa2/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/rgrep/build/$(OS)/$(ARCHDIR) all

distclean: clean
	+make -C $(SRCDIR)/rlib/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/rast/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/rpa1/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/rpa2/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/rvm/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/rjs/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/tests/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/testrpa2/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/rgrep/build/$(OS)/$(ARCHDIR) distclean

clean:
	+make -C $(SRCDIR)/rlib/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/rast/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/rpa1/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/rpa2/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/rvm/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/rjs/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/tests/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/testrpa2/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/rgrep/build/$(OS)/$(ARCHDIR) clean

