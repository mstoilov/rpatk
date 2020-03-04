ROOT_DIR = ../../..
include $(ROOT_DIR)/build/unix/config.mk

BUILD_DIR	= $(ROOT_DIR)/build/unix/bin
INCLUDES	+= $(addprefix -I,../../include ../..)
TARGET1		= $(BUILD_DIR)/libgtest_main.a
TARGET2		= $(BUILD_DIR)/libgtest.a


OBJS1= \
	$(BUILD_DIR)/gtest-all.o \
	$(BUILD_DIR)/gtest_main.o \

OBJS2= \
	$(BUILD_DIR)/gtest-all.o \

vpath %.cc ../../src

all: $(BUILD_DIR) $(TARGET1) $(TARGET2)

# auto-generated dependencies:
-include $(BUILD_DIR)/*.d

$(BUILD_DIR)/%.o: %.cc Makefile | $(BUILD_DIR)
	$(CPP) -std=gnu++11 -c $(INCLUDES) $(CFLAGS) -MMD -MF $(@:%.o=%.d) $< -o $@

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR)
	$(CPP) -std=gnu++11 -c $(INCLUDES) $(CXXFLAGS) -MMD -MF $(@:%.o=%.d) $< -o $@

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(INCLUDES) $(CFLAGS) -MMD -MF $(@:%.o=%.d) $< -o $@

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(TARGET1): $(OBJS1:$(BUILD_DIR)/%=$(BUILD_DIR)/%)
	$(AR) -crs $@ $^

$(TARGET2): $(OBJS2:$(BUILD_DIR)/%=$(BUILD_DIR)/%)
	$(AR) -crs $@ $^

clean:
	-rm -f $(TARGET1)
	-rm -f $(TARGET2)	
	-rm -f $(BUILD_DIR)/*.o
	-rm -rf $(BUILD_DIR) 
