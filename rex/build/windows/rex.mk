TARGET=rex.lib
LINK32_OBJS= \
	"$(OUTDIR)\rexcompiler.obj" \
	"$(OUTDIR)\rexdb.obj" \
	"$(OUTDIR)\rexdfa.obj" \
	"$(OUTDIR)\rexdfaconv.obj" \
	"$(OUTDIR)\rexdfasimulator.obj" \
	"$(OUTDIR)\rexfragment.obj" \
	"$(OUTDIR)\rexnfasimulator.obj" \
	"$(OUTDIR)\rexstate.obj" \
	"$(OUTDIR)\rextransition.obj" \


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

SOURCE="$(SRC_DIR)\rexcompiler.c"
"$(OUTDIR)\rexcompiler.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rexdb.c"
"$(OUTDIR)\rexdb.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rexdfa.c"
"$(OUTDIR)\rexdfa.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rexdfaconv.c"
"$(OUTDIR)\rexdfaconv.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rexdfasimulator.c"
"$(OUTDIR)\rexdfasimulator.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rexfragment.c"
"$(OUTDIR)\rexfragment.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rexnfasimulator.c"
"$(OUTDIR)\rexnfasimulator.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rexstate.c"
"$(OUTDIR)\rexstate.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)
    
SOURCE="$(SRC_DIR)\rextransition.c"
"$(OUTDIR)\rextransition.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)
 