all:
	+make -C $(SRCDIR)/rlib/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/rpa/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/rvm/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/rjs/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/tests/testmisc/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/tests/testrpa/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/tests/testrjs/build/$(OS)/$(ARCHDIR) all
	+make -C $(SRCDIR)/rgrep/build/$(OS)/$(ARCHDIR) all

distclean: clean
	+make -C $(SRCDIR)/rlib/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/rpa/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/rvm/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/rjs/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/tests/testmisc/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/tests/testrpa/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/tests/testrjs/build/$(OS)/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/rgrep/build/$(OS)/$(ARCHDIR) distclean

clean:
	+make -C $(SRCDIR)/rlib/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/rpa/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/rvm/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/rjs/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/tests/testmisc/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/tests/testrpa/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/tests/testrjs/build/$(OS)/$(ARCHDIR) clean
	+make -C $(SRCDIR)/rgrep/build/$(OS)/$(ARCHDIR) clean

install:
	+make -C $(SRCDIR)/rlib/build/$(OS)/$(ARCHDIR) install
	+make -C $(SRCDIR)/rpa/build/$(OS)/$(ARCHDIR) install
	+make -C $(SRCDIR)/rvm/build/$(OS)/$(ARCHDIR) install
	+make -C $(SRCDIR)/rgrep/build/$(OS)/$(ARCHDIR) install

uninstall:
	+make -C $(SRCDIR)/rlib/build/$(OS)/$(ARCHDIR) uninstall
	+make -C $(SRCDIR)/rpa/build/$(OS)/$(ARCHDIR) uninstall
	+make -C $(SRCDIR)/rvm/build/$(OS)/$(ARCHDIR) uninstall
	+make -C $(SRCDIR)/rgrep/build/$(OS)/$(ARCHDIR) uninstall
