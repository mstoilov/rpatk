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

$(RPATK_BIN_INSTALL) :
	mkdir -p $(RPATK_BIN_INSTALL)

$(RPATK_LIB_INSTALL) :
	mkdir -p $(RPATK_LIB_INSTALL)

$(RPATK_INC_INSTALL) :
	mkdir -p $(RPATK_INC_INSTALL)

$(RPATK_INC_INSTALL)/rlib :
	mkdir -p $(RPATK_INC_INSTALL)/rlib

$(RPATK_INC_INSTALL)/rvm :
	mkdir -p $(RPATK_INC_INSTALL)/rvm

$(RPATK_INC_INSTALL)/rpa :
	mkdir -p $(RPATK_INC_INSTALL)/rpa

$(RPATK_INC_INSTALL)/rex :
	mkdir -p $(RPATK_INC_INSTALL)/rex

install: $(RPATK_INC_INSTALL) $(RPATK_BIN_INSTALL) $(RPATK_LIB_INSTALL) $(RPATK_INC_INSTALL)/rlib $(RPATK_INC_INSTALL)/rvm $(RPATK_INC_INSTALL)/rpa $(RPATK_INC_INSTALL)/rex
	cp $(ROOT_DIR)/arch/unix/$(ARCHDIR)/rtypes.h $(RPATK_INC_INSTALL)
	+make -C $(ROOT_DIR)/rlib/build/unix/$(ARCHDIR) install
	+make -C $(ROOT_DIR)/rpa/build/unix/$(ARCHDIR) install
	+make -C $(ROOT_DIR)/rvm/build/unix/$(ARCHDIR) install
	+make -C $(ROOT_DIR)/rex/build/unix/$(ARCHDIR) install
	+make -C $(ROOT_DIR)/rpagrep/build/unix/$(ARCHDIR) install
	+make -C $(ROOT_DIR)/rexgrep/build/unix/$(ARCHDIR) install
	+make -C $(ROOT_DIR)/rexcc/build/unix/$(ARCHDIR) install
ifeq ($(RPATK_LDCONFIG), 1)
	ldconfig -n $(RPATK_LIB_INSTALL)
endif

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
