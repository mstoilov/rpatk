OS := $(shell uname | tr "[:upper:]" "[:lower:]")

ifeq ($(ARCH),)
ARCH := $(shell uname -m)
endif

ifeq ($(ARCH), i386)
ARCHDIR := x86
else ifeq ($(ARCH), i586)
ARCHDIR := x86
else ifeq ($(ARCH), i686)
ARCHDIR := x86
else
ARCHDIR := x86_64
endif

all:
	make -C build/$(OS)/$(ARCHDIR)


clean:
	make -C build/$(OS)/$(ARCHDIR) clean


distclean:
	make -C build/$(OS)/$(ARCHDIR) distclean
