ROOT_DIR = ../..
BUILD_DIR = $(ROOT_DIR)/build/unix/bin
include $(ROOT_DIR)/build/unix/config.mk

all:
	+make -C $(ROOT_DIR)/rlib/build/unix all
	+make -C $(ROOT_DIR)/rvm/build/unix all
	+make -C $(ROOT_DIR)/rex/build/unix all
	+make -C $(ROOT_DIR)/rpa/build/unix all
	+make -C $(ROOT_DIR)/rpagrep/build/unix all	
	+make -C $(ROOT_DIR)/rexgrep/build/unix all
	+make -C $(ROOT_DIR)/rexcc/build/unix all
	+make -C $(ROOT_DIR)/googletest/build/unix all
	+make -C $(ROOT_DIR)/unittests/build/unix all
	+make -C $(ROOT_DIR)/tests/testmisc/build/unix all
	+make -C $(ROOT_DIR)/tests/testrpa/build/unix all
	+make -C $(ROOT_DIR)/tests/testrex/build/unix all

clean:
	+make -C $(ROOT_DIR)/rlib/build/unix clean
	+make -C $(ROOT_DIR)/rvm/build/unix clean
	+make -C $(ROOT_DIR)/rex/build/unix clean
	+make -C $(ROOT_DIR)/rpa/build/unix clean
	+make -C $(ROOT_DIR)/rexgrep/build/unix clean
	+make -C $(ROOT_DIR)/rpagrep/build/unix clean	
	+make -C $(ROOT_DIR)/rexcc/build/unix clean
	+make -C $(ROOT_DIR)/googletest/build/unix clean
	+make -C $(ROOT_DIR)/unittests/build/unix clean
	+make -C $(ROOT_DIR)/tests/testmisc/build/unix clean
	+make -C $(ROOT_DIR)/tests/testrpa/build/unix clean
	+make -C $(ROOT_DIR)/tests/testrex/build/unix clean
	
	

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
	cp $(BUILD_DIR)/librlib.a $(RPATK_LIB_INSTALL)
	cp $(BUILD_DIR)/librlib.so.* $(RPATK_LIB_INSTALL)
	cp $(ROOT_DIR)/rlib/*.h $(RPATK_INC_INSTALL)/rlib
	cp $(BUILD_DIR)/librvm.a $(RPATK_LIB_INSTALL)
	cp $(BUILD_DIR)/librvm.so.* $(RPATK_LIB_INSTALL)
	cp $(ROOT_DIR)/rvm/*.h $(RPATK_INC_INSTALL)/rvm
	cp $(BUILD_DIR)/librpa.a $(RPATK_LIB_INSTALL)
	cp $(BUILD_DIR)/librpa.so.* $(RPATK_LIB_INSTALL)
	cp $(ROOT_DIR)/rpa/*.h $(RPATK_INC_INSTALL)/rpa
	cp $(BUILD_DIR)/librex.a $(RPATK_LIB_INSTALL)
	cp $(BUILD_DIR)/librex.so.* $(RPATK_LIB_INSTALL)
	cp $(ROOT_DIR)/rex/*.h $(RPATK_INC_INSTALL)/rex
	cp $(BUILD_DIR)/rpagrep $(RPATK_BIN_INSTALL)
	cp $(BUILD_DIR)/rexgrep $(RPATK_BIN_INSTALL)
	cp $(BUILD_DIR)/rexcc $(RPATK_BIN_INSTALL)
	
	
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
