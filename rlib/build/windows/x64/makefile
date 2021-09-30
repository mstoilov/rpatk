
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

PORT=windows
ARCH=x64
SRC_DIR=..\..\..
RPATK_DIR=..\..\..\..
INCLUDE_DIRS=/I"$(RPATK_DIR)" /I"$(RPATK_DIR)\arch\$(PORT)\$(ARCH)"


!IF "$(BLDCFG)" == "release"

OUTDIR=.\out
CPP_FLAGS=/nologo $(INCLUDE_DIRS) /W3 /EHsc /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /Fo"$(OUTDIR)\\" /FD /c 

!ELSEIF  "$(BLDCFG)" == "debug"

OUTDIR=.\out
CPP_FLAGS=/nologo $(INCLUDE_DIRS) /Zi /W3 /EHsc  /Od /D "WIN32" /D "_DEBUG" /D "DEBUG" /D "R_DEBUG_MEMALLOC" /D "_CONSOLE" /Fo"$(OUTDIR)\\" /FD  /c

!ENDIF

CPP_FLAGS=$(CPP_FLAGS)

!IF "$(DBG_CONSOLE)" == "shut"
CPP_FLAGS=$(CPP_FLAGS) /D "_SHUT_CONSOLE"
!ENDIF


CPP=cl.exe
LINK32=lib.exe
LINK32_FLAGS=/nologo /LTCG /out:"$(OUTDIR)\$(TARGET)"
# LINK32=link.exe
# LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /incremental:no /machine:I386 /out:"$(OUTDIR)\$(TARGET)" 

include "..\rlib.mk"
