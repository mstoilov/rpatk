TARGET=rpa.lib
LINK32_OBJS= \
	"$(OUTDIR)\rpacache.obj" \
	"$(OUTDIR)\rpadbex.obj" \
	"$(OUTDIR)\rpastat.obj" \
	"$(OUTDIR)\rparecord.obj" \
	"$(OUTDIR)\rpavm.obj" \
	"$(OUTDIR)\rpacompiler.obj" \
	"$(OUTDIR)\rpaparser.obj" \
	"$(OUTDIR)\rpaoptimization.obj" \
	"$(OUTDIR)\rpabitmap.obj" \


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

SOURCE="$(SRC_DIR)\rpacache.c"
"$(OUTDIR)\rpacache.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rpadbex.c"
"$(OUTDIR)\rpadbex.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rpastat.c"
"$(OUTDIR)\rpastat.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rparecord.c"
"$(OUTDIR)\rparecord.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rpavm.c"
"$(OUTDIR)\rpavm.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rpacompiler.c"
"$(OUTDIR)\rpacompiler.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rpaparser.c"
"$(OUTDIR)\rpaparser.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rpaoptimization.c"
"$(OUTDIR)\rpaoptimization.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)
    
SOURCE="$(SRC_DIR)\rpabitmap.c"
"$(OUTDIR)\rpabitmap.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)
 