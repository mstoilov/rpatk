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


REXCC=rexcc.exe
PORT=windows
ARCH=i386
SRC_DIR=..\..\..
DEPSRC_DIR=..\..\..\win32
RPATK_DIR=..\..\..\..
REX_LIB="$(RPATK_DIR)\rex\build\$(PORT)\$(ARCH)\out\rex.lib"
RLIB_LIB="$(RPATK_DIR)\rlib\build\$(PORT)\$(ARCH)\out\rlib.lib"
INCLUDE_DIRS=/I"$(SRC_DIR)" /I"$(SRC_DIR)\win32" /I"$(RPATK_DIR)" /I"$(RPATK_DIR)\arch\$(PORT)\$(ARCH)"

!IF "$(BLDCFG)" == "release"

OUTDIR=.\out
CPP_FLAGS=/nologo $(INCLUDE_DIRS)  /ML /W3 /EHsc /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /Fo"$(OUTDIR)\\" /FD /c
LINK_DEBUG=

!ELSEIF  "$(BLDCFG)" == "debug"

OUTDIR=.\out
CPP_FLAGS=/nologo $(INCLUDE_DIRS)  /MLd  /Zi /W3  /EHsc  /Od /D "WIN32" /D "_DEBUG" /D "DEBUG" /D "_CONSOLE" /YX /Fo"$(OUTDIR)\\" /FD  /c
LINK_DEBUG=/DEBUG

!ENDIF

!IF "$(UNICODECFG)" == "no"

CPP_FLAGS=$(CPP_FLAGS) /D "REX_CHAR_TYPE=unsigned char"

!ENDIF


CPP_FLAGS=$(CPP_FLAGS)
LIBS=$(REX_LIB) $(RLIB_LIB)

CPP=cl.exe
# LINK32=lib.exe
# LINK32_FLAGS=/nologo /machine:I386 /out:"$(OUTDIR)\$(TARGET)"
LINK32=link.exe
LINK32_FLAGS=$(LIBS) kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /incremental:no /machine:I386 $(LINK_DEBUG)

REXCC_OBJS= \
	"$(OUTDIR)\rexccmain.obj" \
	"$(OUTDIR)\rexcc.obj" \
	"$(OUTDIR)\rexccdep.obj" \


ALL : "$(OUTDIR)\$(REXCC)"

CLEAN :
	-@erase "$(OUTDIR)\$(REXCC)"
	-@erase $(REXCC_OBJS)
	-@erase *.pdb *.idb *.pch "$(OUTDIR)\*.pdb"
	-@rd /S /Q "$(OUTDIR)"

"$(OUTDIR)" :
	if not exist "$(OUTDIR)\$(NULL)" mkdir "$(OUTDIR)"


"$(OUTDIR)\$(REXCC)" : "$(OUTDIR)" $(REXCC_OBJS)
	$(LINK32) @<<
	$(LINK32_FLAGS) /out:"$(OUTDIR)\$(REXCC)" $(REXCC_OBJS)
<<


SOURCE="$(SRC_DIR)\rexcc.c"
"$(OUTDIR)\rexcc.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(DEPSRC_DIR)\rexccdep.c"
"$(OUTDIR)\rexccdep.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)

SOURCE="$(SRC_DIR)\rexccmain.c"
"$(OUTDIR)\rexccmain.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)
