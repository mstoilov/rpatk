all:
	make -C $(SRCDIR)/rlib/build/$(OS)/$(ARCHDIR) all
	make -C $(SRCDIR)/rvm/build/$(OS)/$(ARCHDIR) all
	make -C $(SRCDIR)/tests/build/$(OS)/$(ARCHDIR) all

distclean: clean
	make -C $(SRCDIR)/rlib/build/$(OS)/$(ARCHDIR) distclean
	make -C $(SRCDIR)/rvm/build/$(OS)/$(ARCHDIR) distclean
	make -C $(SRCDIR)/tests/build/$(OS)/$(ARCHDIR) distclean

clean:
	make -C $(SRCDIR)/rlib/build/$(OS)/$(ARCHDIR) clean
	make -C $(SRCDIR)/rvm/build/$(OS)/$(ARCHDIR) clean
	make -C $(SRCDIR)/tests/build/$(OS)/$(ARCHDIR) clean
