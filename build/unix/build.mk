all:
	+make -C $(SRCDIR)/rlib/build/unix/$(ARCHDIR) all
	+make -C $(SRCDIR)/rpa/build/unix/$(ARCHDIR) all
	+make -C $(SRCDIR)/rvm/build/unix/$(ARCHDIR) all
	+make -C $(SRCDIR)/rgrep/build/unix/$(ARCHDIR) all
	+make -C $(SRCDIR)/tests/testmisc/build/unix/$(ARCHDIR) all
	+make -C $(SRCDIR)/tests/testrpa/build/unix/$(ARCHDIR) all
ifeq ($(OS), linux)
	+make -C $(SRCDIR)/rjs/build/unix/$(ARCHDIR) all
	+make -C $(SRCDIR)/tests/testrjs/build/unix/$(ARCHDIR) all
endif

distclean: clean
	+make -C $(SRCDIR)/rlib/build/unix/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/rpa/build/unix/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/rvm/build/unix/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/rjs/build/unix/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/tests/testmisc/build/unix/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/tests/testrpa/build/unix/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/tests/testrjs/build/unix/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/rgrep/build/unix/$(ARCHDIR) distclean
	+make -C $(SRCDIR)/rjs/build/unix/$(ARCHDIR) distclean


clean:
	+make -C $(SRCDIR)/rlib/build/unix/$(ARCHDIR) clean
	+make -C $(SRCDIR)/rpa/build/unix/$(ARCHDIR) clean
	+make -C $(SRCDIR)/rvm/build/unix/$(ARCHDIR) clean
	+make -C $(SRCDIR)/tests/testmisc/build/unix/$(ARCHDIR) clean
	+make -C $(SRCDIR)/tests/testrpa/build/unix/$(ARCHDIR) clean
	+make -C $(SRCDIR)/tests/testrjs/build/unix/$(ARCHDIR) clean
	+make -C $(SRCDIR)/rgrep/build/unix/$(ARCHDIR) clean
	+make -C $(SRCDIR)/rjs/build/unix/$(ARCHDIR) clean


$(RPATK_INC_INSTALL) :
	mkdir $(RPATK_INC_INSTALL)

$(RPATK_INC_INSTALL)/rlib :
	mkdir $(RPATK_INC_INSTALL)/rlib

$(RPATK_INC_INSTALL)/rvm :
	mkdir $(RPATK_INC_INSTALL)/rvm

$(RPATK_INC_INSTALL)/rpa :
	mkdir $(RPATK_INC_INSTALL)/rpa

install: $(RPATK_INC_INSTALL) $(RPATK_INC_INSTALL)/rlib $(RPATK_INC_INSTALL)/rvm $(RPATK_INC_INSTALL)/rpa
	cp $(SRCDIR)/arch/unix/$(ARCHDIR)/rtypes.h $(RPATK_INC_INSTALL)
	+make -C $(SRCDIR)/rlib/build/unix/$(ARCHDIR) install
	+make -C $(SRCDIR)/rpa/build/unix/$(ARCHDIR) install
	+make -C $(SRCDIR)/rvm/build/unix/$(ARCHDIR) install
	+make -C $(SRCDIR)/rgrep/build/unix/$(ARCHDIR) install
	ldconfig -n $(RTK_LIB_INSTALL)

uninstall:
	+make -C $(SRCDIR)/rlib/build/unix/$(ARCHDIR) uninstall
	+make -C $(SRCDIR)/rpa/build/unix/$(ARCHDIR) uninstall
	+make -C $(SRCDIR)/rvm/build/unix/$(ARCHDIR) uninstall
	+make -C $(SRCDIR)/rgrep/build/unix/$(ARCHDIR) uninstall
	-rm -rf $(RPATK_INC_INSTALL)/rlib
	-rm -rf $(RPATK_INC_INSTALL)/rvm
	-rm -rf $(RPATK_INC_INSTALL)/rpa
	-rm -rf $(RPATK_INC_INSTALL)
