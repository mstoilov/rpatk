
!IF "$(CFG)" == ""
CFG=debug
!ENDIF 

!IF "$(CFG)" != "release" && "$(CFG)" != "debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE nmake /f makefile CFG=debug
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "release" ( Win32 x86 release build )
!MESSAGE "debug"   ( Win32 x86 debug build)
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 


!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF


TARGET=rlib.lib
PORT=windows
SRC_DIR=..\..\..
RPATK_DIR=..\..\..\..
INCLUDE_DIRS=/I"$(RPATK_DIR)" /I"$(RPATK_DIR)\arch\$(PORT)\i386"


!IF "$(CFG)" == "release"

OUTDIR=.\out
CPP_FLAGS=/nologo $(INCLUDE_DIRS) /Zp1 /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /Fo"$(OUTDIR)\\" /FD /c 

!ELSEIF  "$(CFG)" == "debug"

OUTDIR=.\out
CPP_FLAGS=/nologo $(INCLUDE_DIRS) /Zp1 /MLd  /Zi /W3  /GX  /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /Fo"$(OUTDIR)\\" /FD  /c

!ENDIF

CPP_FLAGS=$(CPP_FLAGS) $(MCS_CFLAGS)

!IF "$(DBG_CONSOLE)" == "shut"
CPP_FLAGS=$(CPP_FLAGS) /D "_SHUT_CONSOLE"
!ENDIF


CPP=cl.exe
LINK32=lib.exe
LINK32_FLAGS=/nologo /machine:I386 /out:"$(OUTDIR)\$(TARGET)"
# LINK32=link.exe
# LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /incremental:no /machine:I386 /out:"$(OUTDIR)\$(TARGET)" 
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

