dnl $Id$
dnl config.m4 for extension prpa

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(prpa, for prpa support,
dnl Make sure that the comment is aligned:
[  --with-prpa             Include prpa support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(prpa, whether to enable prpa support,
dnl Make sure that the comment is aligned:
dnl [  --enable-prpa           Enable prpa support])


PHP_ARG_WITH(rpatk, path to the rpatk dir,
[  --with-rpatk              Path to the RPATK base dir])

if test "$PHP_PRPA" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-rpatk -> check with-path
  SEARCH_PATH="/usr/local /usr ../../.."     # you might want to change this
  SEARCH_FOR="rpa/rpadbex.h"       		 # you most likely want to change this
  AC_MSG_CHECKING([for $PHP_RPATK/$SEARCH_FOR files in default path])
  if test -r $PHP_RPATK/$SEARCH_FOR; then # path given as parameter
    AC_MSG_RESULT([found])
    RPATK_DIR=$PHP_RPATK
  else # search default path list
    AC_MSG_RESULT([not found])
    AC_MSG_CHECKING([for rpatk/$SEARCH_FOR in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/rpatk/$SEARCH_FOR; then
        RPATK_DIR=$i/rpatk
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
  if test -z "$RPATK_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([RPATK dir not found in $SEARCH_PATH])
  fi

  OS_DIR=linux
  MACH=`uname -m`
  if [[ $MACH == "i486" ]]; then MACH=i386; echo "MACH set to: $MACH"; fi
  if [[ $MACH == "i586" ]]; then MACH=i386; echo "MACH set to: $MACH"; fi
  if [[ $MACH == "i686" ]]; then MACH=i386; echo "MACH set to: $MACH"; fi
  ARCH_DIR=$RPATK_DIR/arch/$OS_DIR/$MACH
  RLIB_DIR=$RPATK_DIR/rlib
  RVM_DIR=$RPATK_DIR/rvm
  RPA_DIR=$RPATK_DIR/rpa
  RLIB_LIBDIR=$RPATK_DIR/rlib/build/$OS_DIR/$MACH/out
  RVM_LIBDIR=$RPATK_DIR/rvm/build/$OS_DIR/$MACH/out
  RPA_LIBDIR=$RPATK_DIR/rpa/build/$OS_DIR/$MACH/out


  AC_MSG_RESULT([RPATK dir is: $RPATK_DIR])
  AC_MSG_RESULT([ARCH dir is: $ARCH_DIR])
  AC_MSG_RESULT([RLIB dir is: $RLIB_DIR])
  AC_MSG_RESULT([RVM dir is: $RVM_DIR])
  AC_MSG_RESULT([RPA dir is: $RPA_DIR])
  AC_MSG_RESULT([RLIB lib dir is: $RLIB_LIBDIR])
  AC_MSG_RESULT([RVM lib dir is: $RVM_LIBDIR])
  AC_MSG_RESULT([RPA lib dir is: $RPA_LIBDIR])


  # --with-rpatk -> add include path
  PHP_ADD_INCLUDE($RPATK_DIR)
  PHP_ADD_INCLUDE($ARCH_DIR)
  PHP_ADD_INCLUDE($RLIB_DIR)
  PHP_ADD_INCLUDE($RVM_DIR)
  PHP_ADD_INCLUDE($RPA_DIR)

  dnl # --with-prpa -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/prpa.h"  # you most likely want to change this
  dnl if test -r $PHP_PRPA/$SEARCH_FOR; then # path given as parameter
  dnl   PRPA_DIR=$PHP_PRPA
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for prpa files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       PRPA_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$PRPA_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the prpa distribution])
  dnl fi

  dnl # --with-rpatk -> check for lib and symbol presence
  LIBNAME=rpa # you may want to change this
  LIBSYMBOL=rpa_dbex_version #r_utf8_mbtowc # you most likely want to change this 

  PRPA_SHARED_LIBADD=""
  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $RPA_LIBDIR, PRPA_SHARED_LIBADD)
    AC_DEFINE(HAVE_RPALIB,1,[ ])
  ],[
    AC_MSG_RESULT([RPA lib not found])
  ],[
    -L$RPA_LIBDIR  -L$RVM_LIBDIR -L$RLIB_LIBDIR -lrvm -lrlib  -lrpa  -lm
  ])

  PHP_ADD_LIBRARY_WITH_PATH(rlib, $RLIB_LIBDIR, PRPA_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(rvm, $RVM_LIBDIR, PRPA_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(rpa, $RPA_LIBDIR, PRPA_SHARED_LIBADD)

  PHP_SUBST(PRPA_SHARED_LIBADD)



  PHP_NEW_EXTENSION(prpa, prpa.c, $ext_shared)
fi
