RJS_OBJECTS= \
	"$(OUTDIR)/rjs.res" \
	"$(OUTDIR)/rjs.obj" \
	"$(OUTDIR)/rjsparser.obj" \
	"$(OUTDIR)/rjscompiler.obj" \
	"$(OUTDIR)/rjsfile.obj" \
	"$(OUTDIR)/rjsrules.obj" \
	"$(OUTDIR)\rvmoperator.obj" \
	"$(OUTDIR)\rvmoperatorbin.obj" \
	"$(OUTDIR)\rvmoperatoradd.obj" \
	"$(OUTDIR)\rvmoperatorand.obj" \
	"$(OUTDIR)\rvmoperatoreq.obj" \
	"$(OUTDIR)\rvmoperatornoteq.obj" \
	"$(OUTDIR)\rvmoperatorlogicor.obj" \
	"$(OUTDIR)\rvmoperatorlogicand.obj" \
	"$(OUTDIR)\rvmoperatorlogicnot.obj" \
	"$(OUTDIR)\rvmoperatorless.obj" \
	"$(OUTDIR)\rvmoperatorlesseq.obj" \
	"$(OUTDIR)\rvmoperatorgreater.obj" \
	"$(OUTDIR)\rvmoperatorgreatereq.obj" \
	"$(OUTDIR)\rvmoperatorxor.obj" \
	"$(OUTDIR)\rvmoperatoror.obj" \
	"$(OUTDIR)\rvmoperatorcmp.obj" \
	"$(OUTDIR)\rvmoperatorcmn.obj" \
	"$(OUTDIR)\rvmoperatorlsl.obj" \
	"$(OUTDIR)\rvmoperatorlsr.obj" \
	"$(OUTDIR)\rvmoperatorlsru.obj" \
	"$(OUTDIR)\rvmoperatorcast.obj" \
	"$(OUTDIR)\rvmoperatorcat.obj" \
	"$(OUTDIR)\rvmoperatorsub.obj" \
	"$(OUTDIR)\rvmoperatormul.obj" \
	"$(OUTDIR)\rvmoperatordiv.obj" \
	"$(OUTDIR)\rvmoperatormod.obj" \
	"$(OUTDIR)\rvmoperatornot.obj" \

	
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

SOURCE="$(SRC_DIR)\rvmoperator.c"
"$(OUTDIR)\rvmoperator.obj" : $(SOURCE) $(OUTDIR)
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatorbin.c"
"$(OUTDIR)\rvmoperatorbin.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatoradd.c"
"$(OUTDIR)\rvmoperatoradd.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatorand.c"
"$(OUTDIR)\rvmoperatorand.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatoreq.c"
"$(OUTDIR)\rvmoperatoreq.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatornoteq.c"
"$(OUTDIR)\rvmoperatornoteq.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatorlogicor.c"
"$(OUTDIR)\rvmoperatorlogicor.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatorlogicand.c"
"$(OUTDIR)\rvmoperatorlogicand.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatorlogicnot.c"
"$(OUTDIR)\rvmoperatorlogicnot.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatorless.c"
"$(OUTDIR)\rvmoperatorless.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatorlesseq.c"
"$(OUTDIR)\rvmoperatorlesseq.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatorgreater.c"
"$(OUTDIR)\rvmoperatorgreater.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatorgreatereq.c"
"$(OUTDIR)\rvmoperatorgreatereq.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatorxor.c"
"$(OUTDIR)\rvmoperatorxor.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatoror.c"
"$(OUTDIR)\rvmoperatoror.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatorcmp.c"
"$(OUTDIR)\rvmoperatorcmp.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatorcmn.c"
"$(OUTDIR)\rvmoperatorcmn.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatorlsl.c"
"$(OUTDIR)\rvmoperatorlsl.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatorlsr.c"
"$(OUTDIR)\rvmoperatorlsr.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatorlsru.c"
"$(OUTDIR)\rvmoperatorlsru.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatorcast.c"
"$(OUTDIR)\rvmoperatorcast.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatorcat.c"
"$(OUTDIR)\rvmoperatorcat.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatorsub.c"
"$(OUTDIR)\rvmoperatorsub.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatormul.c"
"$(OUTDIR)\rvmoperatormul.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatordiv.c"
"$(OUTDIR)\rvmoperatordiv.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatormod.c"
"$(OUTDIR)\rvmoperatormod.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rvmoperatornot.c"
"$(OUTDIR)\rvmoperatornot.obj" : $(SOURCE) $(OUTDIR)
    $(CPP) $(CPP_FLAGS) $(SOURCE)


SOURCE="$(DEPSRC_DIR)\rjs.rc"
"$(OUTDIR)\rjs.res" : $(SOURCE) "$(OUTDIR)"
	$(RC) $(RC_FLAGS) /r /fo "$(OUTDIR)\rjs.res" $(SOURCE)
