
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


TARGET=rpa.lib
PORT=windows
SRC_DIR=..\..\..
RPATK_DIR=..\..\..\..
INCLUDE_DIRS=/I"$(RPATK_DIR)" /I"$(RPATK_DIR)\arch\$(PORT)\i386"


!IF "$(CFG)" == "release"

OUTDIR=.\out
CPP_FLAGS=/nologo $(INCLUDE_DIRS) /Zp1 /W3 /EHsc /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /Fo"$(OUTDIR)\\" /FD /c 

!ELSEIF  "$(CFG)" == "debug"

OUTDIR=.\out
CPP_FLAGS=/nologo $(INCLUDE_DIRS) /Zp1 /Zi /W3 /EHsc  /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /Fo"$(OUTDIR)\\" /FD  /c

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
	"$(OUTDIR)\rpacache.obj" \
	"$(OUTDIR)\rpadbex.obj" \
	"$(OUTDIR)\rpastat.obj" \
	"$(OUTDIR)\rparecord.obj" \
	"$(OUTDIR)\rpavm.obj" \
	"$(OUTDIR)\rpacompiler.obj" \
	"$(OUTDIR)\rpaparser.obj" \
	"$(OUTDIR)\rpaoptimization.obj" \


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
