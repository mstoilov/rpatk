RJS_OBJECTS= \
	"$(OUTDIR)/rjs.res" \
	"$(OUTDIR)/rjs.obj" \
	"$(OUTDIR)/rjsparser.obj" \
	"$(OUTDIR)/rjscompiler.obj" \
	"$(OUTDIR)/rjsfile.obj" \
	"$(OUTDIR)/rjsrules.obj"

	
RJSEXEC_OBJECTS =	\
	"$(OUTDIR)\rjsexec.obj" \


ALL : "$(OUTDIR)\$(RJSEXEC)"

CLEAN :
	-@erase "$(OUTDIR)\$(RJSEXEC)"
	-@erase $(RJS_OBJECTS)
	-@erase *.pdb *.idb *.pch "$(OUTDIR)\*.pdb"
	-@rd /S /Q "$(OUTDIR)"

"$(OUTDIR)" :
	if not exist "$(OUTDIR)\$(NULL)" mkdir "$(OUTDIR)"


"$(OUTDIR)\$(RJSEXEC)" : "$(OUTDIR)" $(RJSEXEC_OBJECTS) $(RJS_OBJECTS)
	$(LINK32) @<<
	$(LINK32_FLAGS) /out:"$(OUTDIR)\$(RJSEXEC)" $(RJSEXEC_OBJECTS) $(RJS_OBJECTS)
<<

SOURCE="$(SRC_DIR)\rjsexec.c"
"$(OUTDIR)\rjsexec.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rjs.c"
"$(OUTDIR)\rjs.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rjsparser.c"
"$(OUTDIR)\rjsparser.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rjscompiler.c"
"$(OUTDIR)\rjscompiler.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(DEPSRC_DIR)\rjsfile.c"
"$(OUTDIR)\rjsfile.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(DEPSRC_DIR)\rjsrules.c"
"$(OUTDIR)\rjsrules.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(DEPSRC_DIR)\rjs.rc"
"$(OUTDIR)\rjs.res" : $(SOURCE) "$(OUTDIR)"
	$(RC) $(RC_FLAGS) /r /fo "$(OUTDIR)\rjs.res" $(SOURCE)
