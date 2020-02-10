ROOT_DIR = ..
include $(ROOT_DIR)/unix/config.mk

all:
	+make -C $(ROOT_DIR)/rlib/unix all
	+make -C $(ROOT_DIR)/rpa/unix all
	+make -C $(ROOT_DIR)/rvm/unix all
	+make -C $(ROOT_DIR)/rex/unix all
	+make -C $(ROOT_DIR)/rpagrep/unix all
	+make -C $(ROOT_DIR)/rexgrep/unix all
	+make -C $(ROOT_DIR)/rexcc/unix all
	+make -C $(ROOT_DIR)/tests/testmisc/unix all
	+make -C $(ROOT_DIR)/tests/testrex/unix all
	+make -C $(ROOT_DIR)/tests/testrpa/unix all

clean:
	+make -C $(ROOT_DIR)/rlib/unix clean
	+make -C $(ROOT_DIR)/rpa/unix clean
	+make -C $(ROOT_DIR)/rvm/unix clean
	+make -C $(ROOT_DIR)/rex/unix clean
	+make -C $(ROOT_DIR)/tests/testmisc/unix clean
	+make -C $(ROOT_DIR)/tests/testrpa/unix clean
	+make -C $(ROOT_DIR)/tests/testrex/unix clean
	+make -C $(ROOT_DIR)/tests/testrjs/unix clean	
	+make -C $(ROOT_DIR)/rpagrep/unix clean
	+make -C $(ROOT_DIR)/rexgrep/unix clean
	+make -C $(ROOT_DIR)/rexcc/unix clean
	+make -C $(ROOT_DIR)/rexcc/unix clean
	+make -C $(ROOT_DIR)/rjs/unix clean
	

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
	cp $(ROOT_DIR)/arch/unix/rtypes.h $(RPATK_INC_INSTALL)
	cp $(ROOT_DIR)/rlib/unix/bin/*.a $(RPATK_LIB_INSTALL)
	cp $(ROOT_DIR)/rlib/unix/bin/*.so.* $(RPATK_LIB_INSTALL)
	cp $(ROOT_DIR)/rlib/*.h $(RPATK_INC_INSTALL)/rlib
	cp $(ROOT_DIR)/rvm/unix/bin/*.a $(RPATK_LIB_INSTALL)
	cp $(ROOT_DIR)/rvm/unix/bin/*.so.* $(RPATK_LIB_INSTALL)
	cp $(ROOT_DIR)/rvm/*.h $(RPATK_INC_INSTALL)/rvm
	cp $(ROOT_DIR)/rpa/unix/bin/*.a $(RPATK_LIB_INSTALL)
	cp $(ROOT_DIR)/rpa/unix/bin/*.so.* $(RPATK_LIB_INSTALL)
	cp $(ROOT_DIR)/rpa/*.h $(RPATK_INC_INSTALL)/rpa
	cp $(ROOT_DIR)/rex/unix/bin/*.a $(RPATK_LIB_INSTALL)
	cp $(ROOT_DIR)/rex/unix/bin/*.so.* $(RPATK_LIB_INSTALL)
	cp $(ROOT_DIR)/rex/*.h $(RPATK_INC_INSTALL)/rex
	cp $(ROOT_DIR)/rpagrep/unix/bin/rpagrep $(RPATK_BIN_INSTALL)
	cp $(ROOT_DIR)/rexgrep/unix/bin/rexgrep $(RPATK_BIN_INSTALL)
	cp $(ROOT_DIR)/rexcc/unix/bin/rexcc $(RPATK_BIN_INSTALL)
	
	
ifeq ($(RPATK_LDCONFIG), 1)
	ldconfig -n $(RPATK_LIB_INSTALL)
endif


uninstall:
	-rm -rf $(RPATK_BIN_INSTALL)/rpagrep
	-rm -rf $(RPATK_BIN_INSTALL)/rexgrep
	-rm -rf $(RPATK_BIN_INSTALL)/rexcc
	-rm -rf $(RPATK_LIB_INSTALL)/librlib.*
	-rm -rf $(RPATK_LIB_INSTALL)/librex.*
	-rm -rf $(RPATK_LIB_INSTALL)/librvm.*
	-rm -rf $(RPATK_LIB_INSTALL)/librpa.*
	-rm -rf $(RPATK_LIB_INSTALL)/rlib
	-rm -rf $(RPATK_LIB_INSTALL)/rvm
	-rm -rf $(RPATK_LIB_INSTALL)/rpa
	-rm -rf $(RPATK_LIB_INSTALL)/rex
	-rm -rf $(RPATK_INC_INSTALL)/rlib
	-rm -rf $(RPATK_INC_INSTALL)/rvm
	-rm -rf $(RPATK_INC_INSTALL)/rpa
	-rm -rf $(RPATK_INC_INSTALL)/rex
	-rm -rf $(RPATK_INC_INSTALL)
