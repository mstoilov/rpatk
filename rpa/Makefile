all: $(OUTDIR) $(BNF_LIB)
	make -C make/unix

distclean: clean
	make -C make/unix clean


clean:
	make -C make/unix distclean


sdk:
	make -C make/unix sdk

install:
	make -C make/unix install

uninstall:
	make -C make/unix uninstall
