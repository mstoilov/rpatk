all:
	+make -C $(ROOT_DIR)/rlib/build/unix/$(ARCHDIR) all
	+make -C $(ROOT_DIR)/rpa/build/unix/$(ARCHDIR) all
	+make -C $(ROOT_DIR)/rvm/build/unix/$(ARCHDIR) all
	+make -C $(ROOT_DIR)/rex/build/unix/$(ARCHDIR) all
	+make -C $(ROOT_DIR)/rpagrep/build/unix/$(ARCHDIR) all
	+make -C $(ROOT_DIR)/rexgrep/build/unix/$(ARCHDIR) all
	+make -C $(ROOT_DIR)/rexcc/build/unix/$(ARCHDIR) all
	+make -C $(ROOT_DIR)/tests/testmisc/build/unix/$(ARCHDIR) all
	+make -C $(ROOT_DIR)/tests/testrpa/build/unix/$(ARCHDIR) all
	+make -C $(ROOT_DIR)/tests/testrex/build/unix/$(ARCHDIR) all
	+make -C $(ROOT_DIR)/rjs/build/unix/$(ARCHDIR) all
ifeq ($(OS), linux)
	+make -C $(ROOT_DIR)/tests/testrjs/build/unix/$(ARCHDIR) all
endif

distclean: clean
	+make -C $(ROOT_DIR)/rlib/build/unix/$(ARCHDIR) distclean
	+make -C $(ROOT_DIR)/rpa/build/unix/$(ARCHDIR) distclean
	+make -C $(ROOT_DIR)/rvm/build/unix/$(ARCHDIR) distclean
	+make -C $(ROOT_DIR)/rex/build/unix/$(ARCHDIR) distclean
	+make -C $(ROOT_DIR)/rjs/build/unix/$(ARCHDIR) distclean
	+make -C $(ROOT_DIR)/tests/testmisc/build/unix/$(ARCHDIR) distclean
	+make -C $(ROOT_DIR)/tests/testrpa/build/unix/$(ARCHDIR) distclean
	+make -C $(ROOT_DIR)/tests/testrex/build/unix/$(ARCHDIR) distclean
	+make -C $(ROOT_DIR)/tests/testrjs/build/unix/$(ARCHDIR) distclean
	+make -C $(ROOT_DIR)/rpagrep/build/unix/$(ARCHDIR) distclean
	+make -C $(ROOT_DIR)/rexgrep/build/unix/$(ARCHDIR) distclean
	+make -C $(ROOT_DIR)/rexcc/build/unix/$(ARCHDIR) distclean
	+make -C $(ROOT_DIR)/rjs/build/unix/$(ARCHDIR) distclean


clean:
	+make -C $(ROOT_DIR)/rlib/build/unix/$(ARCHDIR) clean
	+make -C $(ROOT_DIR)/rpa/build/unix/$(ARCHDIR) clean
	+make -C $(ROOT_DIR)/rvm/build/unix/$(ARCHDIR) clean
	+make -C $(ROOT_DIR)/rex/build/unix/$(ARCHDIR) clean
	+make -C $(ROOT_DIR)/tests/testmisc/build/unix/$(ARCHDIR) clean
	+make -C $(ROOT_DIR)/tests/testrpa/build/unix/$(ARCHDIR) clean
	+make -C $(ROOT_DIR)/tests/testrex/build/unix/$(ARCHDIR) clean
	+make -C $(ROOT_DIR)/tests/testrjs/build/unix/$(ARCHDIR) clean
	+make -C $(ROOT_DIR)/rpagrep/build/unix/$(ARCHDIR) clean
	+make -C $(ROOT_DIR)/rexgrep/build/unix/$(ARCHDIR) clean
	+make -C $(ROOT_DIR)/rexcc/build/unix/$(ARCHDIR) clean
	+make -C $(ROOT_DIR)/rjs/build/unix/$(ARCHDIR) clean


$(RPATK_INC_INSTALL) :
	mkdir $(RPATK_INC_INSTALL)

$(RPATK_INC_INSTALL)/rlib :
	mkdir $(RPATK_INC_INSTALL)/rlib

$(RPATK_INC_INSTALL)/rvm :
	mkdir $(RPATK_INC_INSTALL)/rvm

$(RPATK_INC_INSTALL)/rpa :
	mkdir $(RPATK_INC_INSTALL)/rpa

$(RPATK_INC_INSTALL)/rex :
	mkdir $(RPATK_INC_INSTALL)/rex

install: $(RPATK_INC_INSTALL) $(RPATK_INC_INSTALL)/rlib $(RPATK_INC_INSTALL)/rvm $(RPATK_INC_INSTALL)/rpa $(RPATK_INC_INSTALL)/rex
	cp $(ROOT_DIR)/arch/unix/$(ARCHDIR)/rtypes.h $(RPATK_INC_INSTALL)
	+make -C $(ROOT_DIR)/rlib/build/unix/$(ARCHDIR) install
	+make -C $(ROOT_DIR)/rpa/build/unix/$(ARCHDIR) install
	+make -C $(ROOT_DIR)/rvm/build/unix/$(ARCHDIR) install
	+make -C $(ROOT_DIR)/rex/build/unix/$(ARCHDIR) install
	+make -C $(ROOT_DIR)/rpagrep/build/unix/$(ARCHDIR) install
	+make -C $(ROOT_DIR)/rexgrep/build/unix/$(ARCHDIR) install
	+make -C $(ROOT_DIR)/rexcc/build/unix/$(ARCHDIR) install
	ldconfig -n $(RTK_LIB_INSTALL)

uninstall:
	+make -C $(ROOT_DIR)/rlib/build/unix/$(ARCHDIR) uninstall
	+make -C $(ROOT_DIR)/rpa/build/unix/$(ARCHDIR) uninstall
	+make -C $(ROOT_DIR)/rvm/build/unix/$(ARCHDIR) uninstall
	+make -C $(ROOT_DIR)/rpagrep/build/unix/$(ARCHDIR) uninstall
	+make -C $(ROOT_DIR)/rexgrep/build/unix/$(ARCHDIR) uninstall
	+make -C $(ROOT_DIR)/rexcc/build/unix/$(ARCHDIR) uninstall
	-rm -rf $(RPATK_INC_INSTALL)/rlib
	-rm -rf $(RPATK_INC_INSTALL)/rvm
	-rm -rf $(RPATK_INC_INSTALL)/rpa
	-rm -rf $(RPATK_INC_INSTALL)/rex
	-rm -rf $(RPATK_INC_INSTALL)
