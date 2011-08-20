TARGET=rvm.lib
LINK32_OBJS= \
	"$(OUTDIR)\rvmcodemap.obj" \
	"$(OUTDIR)\rvmrelocmap.obj" \
	"$(OUTDIR)\rvmcodegen.obj" \
	"$(OUTDIR)\rvmreg.obj" \
	"$(OUTDIR)\rvmscope.obj" \
	"$(OUTDIR)\rvmcpu.obj" \


ALL : "$(OUTDIR)\$(TARGET)"

CLEAN :
	-@erase "$(OUTDIR)\$(TARGET)"
	-@erase $(LINK32_OBJS)
	-@erase *.pdb *.idb *.pch
	-@rd /S /Q "$(OUTDIR)"

"$(OUTDIR)" :
	if not exist "$(OUTDIR)\$(NULL)" mkdir "$(OUTDIR)"


"$(OUTDIR)\$(TARGET)" : "$(OUTDIR)" $(LINK32_OBJS)
	$(LINK32) @<<
	$(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(SRC_DIR)\rvmcpu.c"
"$(OUTDIR)\rvmcpu.obj" : $(SOURCE) $(OUTDIR)
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmcodemap.c"
"$(OUTDIR)\rvmcodemap.obj" : $(SOURCE) $(OUTDIR)
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmrelocmap.c"
"$(OUTDIR)\rvmrelocmap.obj" : $(SOURCE) $(OUTDIR)
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmcodegen.c"
"$(OUTDIR)\rvmcodegen.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmreg.c"
"$(OUTDIR)\rvmreg.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmscope.c"
"$(OUTDIR)\rvmscope.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)
