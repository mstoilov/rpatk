ROOT_DIR = ../..
include $(ROOT_DIR)/unix/config.mk

OUTDIR = bin
SO_VERSION = 2.0
BASENAME = rlib
LIB_NAME = lib$(BASENAME).a
SO_NAME = lib$(BASENAME).so
SO_NAME_VERSION = $(SO_NAME).$(SO_VERSION)
LIB_TARGET = $(OUTDIR)/$(LIB_NAME)
SO_TARGET = $(OUTDIR)/$(SO_NAME_VERSION)
CFLAGS += -fPIC
CFLAGS += -I$(ROOT_DIR) -I$(ROOT_DIR)/arch/unix/$(ARCH)

LIBS += -lpthread -lm
LDFLAGS += $(LIBS)

OBJECTS += $(addprefix $(OUTDIR)/, $(patsubst %.c,%.o,$(notdir $(wildcard ../*.c))))
vpath %.c ../


ifeq ($(OS), linux)
all: $(LIB_TARGET) $(SO_TARGET)
else
all: $(LIB_TARGET)
endif


$(OUTDIR)/%.o: %.c Makefile | $(OUTDIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(OUTDIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(LIB_TARGET): $(OBJECTS) | $(OUTDIR)
	$(AR) -cr $@ $^

$(SO_TARGET): $(OBJECTS) | $(OUTDIR)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(SO_NAME) -o $@ $^


$(OUTDIR):
	mkdir $@		


clean:
	-rm -f *~
	-rm -rf $(OUTDIR)
