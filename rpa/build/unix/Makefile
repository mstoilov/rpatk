ROOT_DIR = ../../..
include $(ROOT_DIR)/build/unix/config.mk

OUTDIR = $(ROOT_DIR)/build/unix/bin
RLIB_SRCDIR = $(ROOT_DIR)/rlib
RVM_SRCDIR = $(ROOT_DIR)/rvm
SO_VERSION = 2.0
BASENAME = librpa
LIB_NAME = $(BASENAME).a
SO_NAME = $(BASENAME).so
SO_NAME_VERSION = $(SO_NAME).$(SO_VERSION)
LIB_TARGET = $(OUTDIR)/$(LIB_NAME)
SO_TARGET = $(OUTDIR)/$(SO_NAME_VERSION)
CFLAGS += -fPIC
CFLAGS += -I$(ROOT_DIR) -I$(ROOT_DIR)/arch/unix -I$(RVM_SRCDIR) -I$(RLIB_SRCDIR)

LIBS += -lpthread -lm
LDFLAGS += $(LIBS)

OBJECTS += $(addprefix $(OUTDIR)/, $(patsubst %.c,%.o,$(notdir $(wildcard ../../*.c))))
vpath %.c ../..


ifeq ($(OS), linux)
all: $(LIB_TARGET) $(SO_TARGET)
else
all: $(LIB_TARGET)
endif

-include $(OUTDIR)/*.d

$(OUTDIR)/%.o: %.c Makefile | $(OUTDIR)
	$(CC) -c $(CFLAGS) -MMD -MF $(@:%.o=%.d) $< -o $@

$(LIB_TARGET): $(OBJECTS) | $(OUTDIR)
	$(AR) -cr $@ $^

$(SO_TARGET): $(OBJECTS) | $(OUTDIR)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(SO_NAME) -o $@ $^


$(OUTDIR):
	mkdir $@		


clean:
	-rm -f *~
	-rm -rf $(OBJECTS)
	-rm -rf $(LIB_TARGET)
	-rm -rf $(LIB_TARGET)

