!IF "$(BLDCFG)" == ""
BLDCFG=debug
!ENDIF 

!IF "$(BLDCFG)" != "release" && "$(BLDCFG)" != "debug"
!MESSAGE Invalid configuration "$(BLDCFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro BLDCFG on the command line. For example:
!MESSAGE 
!MESSAGE nmake /f makefile BLDCFG=debug
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


RGREP=rpagrep.exe
PORT=windows
ARCH=i386
SRC_DIR=..\..\..
DEPSRC_DIR=..\..\..\win32
RPATK_DIR=..\..\..\..
RPA_LIB="$(RPATK_DIR)\rpa\build\$(PORT)\$(ARCH)\out\rpa.lib"
RVM_LIB="$(RPATK_DIR)\rvm\build\$(PORT)\$(ARCH)\out\rvm.lib"
RLIB_LIB="$(RPATK_DIR)\rlib\build\$(PORT)\$(ARCH)\out\rlib.lib"
INCLUDE_DIRS=/I"$(SRC_DIR)" /I"$(SRC_DIR)\win32" /I"$(RPATK_DIR)" /I"$(RPATK_DIR)\arch\$(PORT)\$(ARCH)"

!IF "$(BLDCFG)" == "release"

OUTDIR=.\out
CPP_FLAGS=/nologo $(INCLUDE_DIRS)  /ML /W3 /EHsc /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /YX /Fo"$(OUTDIR)\\" /FD /c
LINK_DEBUG=

!ELSEIF  "$(BLDCFG)" == "debug"

OUTDIR=.\out
CPP_FLAGS=/nologo $(INCLUDE_DIRS)  /MLd  /Zi /W3  /EHsc  /Od /D "WIN32" /D "_DEBUG" /D "DEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /YX /Fo"$(OUTDIR)\\" /FD  /c
LINK_DEBUG=/DEBUG

!ENDIF

CPP_FLAGS=$(CPP_FLAGS)
LIBS=$(RPA_LIB) $(RVM_LIB) $(RLIB_LIB)

CPP=cl.exe
# LINK32=lib.exe
# LINK32_FLAGS=/nologo /machine:I386 /out:"$(OUTDIR)\$(TARGET)"
LINK32=link.exe
LINK32_FLAGS=$(LIBS) kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /incremental:no /machine:I386 $(LINK_DEBUG)

RGREP_OBJS= \
	"$(OUTDIR)\rpagrep.obj" \
	"$(OUTDIR)\rpagreputf.obj" \
	"$(OUTDIR)\fsenumwin.obj" \
	"$(OUTDIR)\rpagrepdep.obj" \
	"$(OUTDIR)\main.obj" \


ALL : "$(OUTDIR)\$(RGREP)"

CLEAN :
	-@erase "$(OUTDIR)\$(RGREP)"
	-@erase $(RGREP_OBJS)
	-@erase *.pdb *.idb *.pch "$(OUTDIR)\*.pdb"
	-@rd /S /Q "$(OUTDIR)"

"$(OUTDIR)" :
	if not exist "$(OUTDIR)\$(NULL)" mkdir "$(OUTDIR)"


"$(OUTDIR)\$(RGREP)" : "$(OUTDIR)" $(RGREP_OBJS)
	$(LINK32) @<<
	$(LINK32_FLAGS) /out:"$(OUTDIR)\$(RGREP)" $(RGREP_OBJS)
<<


SOURCE="$(SRC_DIR)\rpagrep.c"
"$(OUTDIR)\rpagrep.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rpagreputf.c"
"$(OUTDIR)\rpagreputf.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(DEPSRC_DIR)\rpagrepdep.c"
"$(OUTDIR)\rpagrepdep.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(DEPSRC_DIR)\fsenumwin.c"
"$(OUTDIR)\fsenumwin.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(DEPSRC_DIR)\main.c"
"$(OUTDIR)\main.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)
