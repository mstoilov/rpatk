ROOT_DIR = ../..
include $(ROOT_DIR)/build/unix/config.mk

OUTDIR = bin
SO_VERSION = 2.0
BASENAME = librjs
LIB_NAME = $(BASENAME).a
SO_NAME = $(BASENAME).so
SO_NAME_VERSION = $(SO_NAME).$(SO_VERSION)
LIB_TARGET = $(OUTDIR)/$(LIB_NAME)
SO_TARGET = $(OUTDIR)/$(SO_NAME_VERSION)
EXE_TARGET = $(OUTDIR)/rjsexec

RLIB_SRCDIR = $(ROOT_DIR)/rlib
RVM_SRCDIR = $(ROOT_DIR)/rvm
RPA_SRCDIR = $(ROOT_DIR)/rpa
RJS_SRCDIR = $(ROOT_DIR)/rjs
CFLAGS += -fPIC
CFLAGS += -I$(ROOT_DIR) -I$(ROOT_DIR)/arch/unix -I$(RVM_SRCDIR) -I$(RLIB_SRCDIR)
LIBS += -L$(RLIB_SRCDIR)/unix/bin -L$(RVM_SRCDIR)/unix/bin -L$(RLIB_SRCDIR)/unix/bin -L$(RPA_SRCDIR)/unix/bin -L$(RJS_SRCDIR)/unix/bin -lrjs -lrpa -lrvm -lrlib -lpthread -lm
LDFLAGS += $(LIBS)
ELFARCH = elf64-x86-64
BINARCH = i386:x86-64
OCFLAGS_TXT = --input binary --output $(ELFARCH) --binary-architecture $(BINARCH)

OBJECTS += $(addprefix $(OUTDIR)/, $(patsubst %.c,%.o,$(notdir $(wildcard ../*.c))))
vpath %.c ../

OBJECTS += $(addprefix $(OUTDIR)/, $(patsubst %.c,%.o,$(notdir $(wildcard *.c))))
vpath %.c .

OBJECTS += $(OUTDIR)/ecma262.o
RJSEXEC_OBJECTS = $(OUTDIR)/rjsexec.o \


ifeq ($(OS), linux)
all: $(LIB_TARGET) $(SO_TARGET) $(EXE_TARGET)
else
all: $(LIB_TARGET)
endif

$(EXE_TARGET) : $(RJSEXEC_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS) -static

$(OUTDIR)/%.o: %.c Makefile | $(OUTDIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(OUTDIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(OUTDIR)/%.o: ../exec/%.c | $(OUTDIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(OUTDIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(LIB_TARGET): $(OBJECTS) | $(OUTDIR)
	$(AR) -cr $@ $^

$(OUTDIR)/%.o: ../%.rpa
	$(OBJCOPY) $(OCFLAGS_TXT)  $< $(OUTDIR)/$*.o

$(SO_TARGET): $(OBJECTS) | $(OUTDIR)
	$(CC) -o $@ $^ -shared -Wl,-soname,$(SO_NAME) 


$(OUTDIR):
	mkdir $@		


clean:
	-rm -f *~
	-rm -rf $(OUTDIR)

