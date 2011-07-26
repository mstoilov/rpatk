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


RJSEXEC=rjsexec.exe
PORT=windows
ARCH=x64
SRC_DIR=..\..\..
DEPSRC_DIR=$(SRC_DIR)\windows
RPATK_DIR=..\..\..\..
RPA_LIB="$(RPATK_DIR)\rpa\build\$(PORT)\$(ARCH)\out\rpa.lib"
RVM_LIB="$(RPATK_DIR)\rvm\build\$(PORT)\$(ARCH)\out\rvm.lib"
RLIB_LIB="$(RPATK_DIR)\rlib\build\$(PORT)\$(ARCH)\out\rlib.lib"
INCLUDE_DIRS=/I"$(SRC_DIR)" /I"$(SRC_DIR)\win32" /I"$(RPATK_DIR)" /I"$(RPATK_DIR)\arch\$(PORT)\$(ARCH)"

!IF "$(BLDCFG)" == "release"

OUTDIR=.\out
CPP_FLAGS=/nologo $(INCLUDE_DIRS) /Zp1 /ML /W3 /EHsc /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /Fo"$(OUTDIR)\\" /FD /c
LINK_DEBUG=

!ELSEIF  "$(BLDCFG)" == "debug"

OUTDIR=.\out
CPP_FLAGS=/nologo $(INCLUDE_DIRS) /Zp1 /MLd  /Zi /W3  /EHsc  /Od /D "WIN32" /D "_DEBUG" /D "DEBUG" /D "_CONSOLE" /YX /Fo"$(OUTDIR)\\" /FD  /c
LINK_DEBUG=/DEBUG

!ENDIF

CPP_FLAGS=$(CPP_FLAGS)
LIBS=$(RPA_LIB) $(RVM_LIB) $(RLIB_LIB)

RC=rc.exe
CPP=cl.exe
# LINK32=lib.exe
# LINK32_FLAGS=/nologo /LTCG /out:"$(OUTDIR)\$(TARGET)"
LINK32=link.exe
LINK32_FLAGS=$(LIBS) kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /incremental:no $(LINK_DEBUG)

include "../rjs.mk"
