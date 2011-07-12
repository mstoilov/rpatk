TARGET=rlib.lib
LINK32_OBJS= \
	"$(OUTDIR)\rref.obj" \
	"$(OUTDIR)\rcharconv.obj" \
	"$(OUTDIR)\robject.obj" \
	"$(OUTDIR)\rgc.obj" \
	"$(OUTDIR)\rmem.obj" \
	"$(OUTDIR)\rmath.obj" \
	"$(OUTDIR)\ratomic.obj" \
	"$(OUTDIR)\rspinlock.obj" \
	"$(OUTDIR)\rharray.obj" \
	"$(OUTDIR)\rcarray.obj" \
	"$(OUTDIR)\rarray.obj" \
	"$(OUTDIR)\rhash.obj" \
	"$(OUTDIR)\rmap.obj" \
	"$(OUTDIR)\rstring.obj" \
	"$(OUTDIR)\rlist.obj" \
	"$(OUTDIR)\rutf.obj" \


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

SOURCE="$(SRC_DIR)\rref.c"
"$(OUTDIR)\rref.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rcharconv.c"
"$(OUTDIR)\rcharconv.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\robject.c"
"$(OUTDIR)\robject.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rgc.c"
"$(OUTDIR)\rgc.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rmem.c"
"$(OUTDIR)\rmem.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rmath.c"
"$(OUTDIR)\rmath.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\ratomic.c"
"$(OUTDIR)\ratomic.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rspinlock.c"
"$(OUTDIR)\rspinlock.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rharray.c"
"$(OUTDIR)\rharray.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rcarray.c"
"$(OUTDIR)\rcarray.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rarray.c"
"$(OUTDIR)\rarray.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)


SOURCE="$(SRC_DIR)\rhash.c"
"$(OUTDIR)\rhash.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)


SOURCE="$(SRC_DIR)\rmap.c"
"$(OUTDIR)\rmap.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)


SOURCE="$(SRC_DIR)\rstring.c"
"$(OUTDIR)\rstring.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)


SOURCE="$(SRC_DIR)\rlist.c"
"$(OUTDIR)\rlist.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rutf.c"
"$(OUTDIR)\rutf.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

