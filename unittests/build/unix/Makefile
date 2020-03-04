ROOT_DIR = ../../..
include $(ROOT_DIR)/build/unix/config.mk

OUTDIR		= $(ROOT_DIR)/build/unix/bin
INCLUDES	+= $(addprefix -I,.. $(ROOT_DIR) $(ROOT_DIR)/arch/unix $(ROOT_DIR)/googletest/include )
LIBS 		+= -lrex -lrlib -lgtest_main -pthread -lm -L$(OUTDIR)
LDFLAGS 	+= $(LIBS)

UNIT_TESTS	= \
	test-robject.cpp \
	test-rstring.cpp \
	test-rarray.cpp \
	test-rcarray.cpp \
	test-rhash.cpp \
	test-rmap.cpp \
	test-rexstate.cpp \
	test-rexdb.cpp \
	


TARGETS		= $(addprefix $(OUTDIR)/,$(notdir $(UNIT_TESTS:.cpp=.unit)))
vpath %.cpp ../..

all: $(TARGETS)

# auto-generated dependencies:
-include $(OUTDIR)/*.d

$(OUTDIR)/%.unit : %.cpp Makefile
	$(CPP) -std=gnu++11 $(INCLUDES) $(CXXFLAGS) -MMD -MF $(@:%.o=%.d) $< -o $@ $(LDFLAGS)

$(OUTDIR):
	mkdir $(OUTDIR)

clean:
	-rm -f $(TARGETS)	

test: all
	@for target in $(TARGETS); do $$target; done

