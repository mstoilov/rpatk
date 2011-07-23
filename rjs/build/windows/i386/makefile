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


RJSEXEC=rjsexec.exe
PORT=windows
ARCH=i386
SRC_DIR=..\..\..
DEPSRC_DIR=$(SRC_DIR)\windows
RPATK_DIR=..\..\..\..
RPA_LIB="$(RPATK_DIR)\rpa\build\$(PORT)\$(ARCH)\out\rpa.lib"
RVM_LIB="$(RPATK_DIR)\rvm\build\$(PORT)\$(ARCH)\out\rvm.lib"
RLIB_LIB="$(RPATK_DIR)\rlib\build\$(PORT)\$(ARCH)\out\rlib.lib"
INCLUDE_DIRS=/I"$(SRC_DIR)" /I"$(SRC_DIR)\win32" /I"$(RPATK_DIR)" /I"$(RPATK_DIR)\arch\$(PORT)\$(ARCH)"

!IF "$(CFG)" == "release"

OUTDIR=.\out
CPP_FLAGS=/nologo $(INCLUDE_DIRS) /Zp1 /ML /W3 /EHsc /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /YX /Fo"$(OUTDIR)\\" /FD /c
LINK_DEBUG=

!ELSEIF  "$(CFG)" == "debug"

OUTDIR=.\out
CPP_FLAGS=/nologo $(INCLUDE_DIRS) /Zp1 /MLd  /Zi /W3  /EHsc  /Od /D "WIN32" /D "_DEBUG" /D "DEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /YX /Fo"$(OUTDIR)\\" /FD  /c
LINK_DEBUG=/DEBUG

!ENDIF

CPP_FLAGS=$(CPP_FLAGS)
LIBS=$(RPA_LIB) $(RVM_LIB) $(RLIB_LIB)

CPP=cl.exe
# LINK32=lib.exe
# LINK32_FLAGS=/nologo /machine:I386 /out:"$(OUTDIR)\$(TARGET)"
LINK32=link.exe
LINK32_FLAGS=$(LIBS) kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /incremental:no /machine:I386 $(LINK_DEBUG)

RJS_OBJECTS= \
	"$(OUTDIR)/rjs.obj" \
	"$(OUTDIR)/rjsparser.obj" \
	"$(OUTDIR)/rjscompiler.obj" \
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

SOURCE="$(DEPSRC_DIR)\rjsexec.c"
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

SOURCE="$(DEPSRC_DIR)\rjsrules.c"
"$(OUTDIR)\rjsrules.obj" : $(SOURCE) "$(OUTDIR)"
	$(CPP) $(CPP_FLAGS) $(SOURCE)
