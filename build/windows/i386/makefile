!IF "$(BLDCFG)" == ""
BLDCFG=release
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


PORT=windows
ARCH=i386
RPATK_DIR=..\..\..

SUBPROJECTS= \
	$(RPATK_DIR)\rlib\build\$(PORT)\$(ARCH)+ \
	$(RPATK_DIR)\rvm\build\$(PORT)\$(ARCH)+ \
	$(RPATK_DIR)\rpa\build\$(PORT)\$(ARCH)+ \
	$(RPATK_DIR)\rex\build\$(PORT)\$(ARCH)+ \
	$(RPATK_DIR)\rpagrep\build\$(PORT)\$(ARCH)+ \
	$(RPATK_DIR)\rexgrep\build\$(PORT)\$(ARCH)+ \
	$(RPATK_DIR)\rexcc\build\$(PORT)\$(ARCH)+ \


ALL : $(SUBPROJECTS)

$(SUBPROJECTS) :
  @echo Building $(@:+=) ...
  @cd $(@:+=)
  @$(MAKE) /NOLOGO /$(MAKEFLAGS)
  @cd $(MAKEDIR)
  @echo Built $(@:+=)

CLEAN : $(SUBPROJECTS:+=-clean)

$(SUBPROJECTS:+=-clean) :
  @echo Building $@ clean ...
  @cd $(@:-clean=)
  @$(MAKE) /NOLOGO /$(MAKEFLAGS) clean
  @cd $(MAKEDIR)
  @echo Built $@

