dnl $Id$
dnl config.m4 for extension prpa

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(prpa, for prpa support,
[  --with-prpa             Include prpa support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(prpa, whether to enable prpa support,
dnl Make sure that the comment is aligned:
dnl [  --enable-prpa           Enable prpa support])

if test "$PHP_PRPA" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-prpa -> check with-path
  MACHDIR=`uname -m 2>/dev/null`
  OS=linux

  SEARCH_PATH="../../arch/$OS/$MACHDIR"     # you might want to change this
  SEARCH_FOR="rtypes.h"       			 # you most likely want to change this
  if test -r $PHP_PRPA/arch/$OS/$MACHDIR/$SEARCH_FOR; then # path given as parameter
     RTYPES_DIR=$PHP_PRPA/arch/$OS/$MACHDIR
  else # search default path list
       AC_MSG_CHECKING([for arch files in default path])
       for i in $SEARCH_PATH; do
       	   if test -r $i/$SEARCH_FOR; then
	      RTYPES_DIR=$i
              AC_MSG_RESULT(found in $i)
       	   fi
	done

  fi

  if test -z "$RTYPES_DIR"; then
     AC_MSG_RESULT([not found])
     AC_MSG_ERROR([arch dir not found in $SEARCH_PATH . Please reinstall the prpa distribution])
  else
     AC_MSG_RESULT([found])
     PHP_ADD_INCLUDE($RTYPES_DIR)
  fi

  SEARCH_PATH="../../rpa"     # you might want to change this
  SEARCH_FOR="rpadbex.h"       			 # you most likely want to change this
  if test -r $PHP_PRPA/$SEARCH_FOR; then # path given as parameter
     PRPA_DIR=$PHP_PRPA
  else # search default path list
       AC_MSG_CHECKING([for prpa files in default path])
       for i in $SEARCH_PATH ; do
       	   if test -r $i/$SEARCH_FOR; then
       	      PRPA_DIR=$i
              AC_MSG_RESULT(found in $i)
       	   fi
	done
  fi

  if test -z "$PRPA_DIR"; then
     AC_MSG_RESULT([not found])
     AC_MSG_ERROR([RPA dir not found. Please reinstall the prpa distribution])
  fi



  dnl # --with-prpa -> add include path
  PHP_ADD_INCLUDE($PRPA_DIR)


  dnl # --with-prpa -> check for lib and symbol presence
  LIBNAME=rpa
  LIBSYMBOL=rpa_dbex_create

  if test -e "$PRPA_DIR/librpasc.so.1.0" ; then
     PHP_ADD_LIBRARY_WITH_PATH(rpasc, $PRPA_DIR, PRPA_SHARED_LIBADD)
  else
     PHP_ADD_LIBRARY_WITH_PATH(rpasx, $PRPA_DIR, PRPA_SHARED_LIBADD)
  fi
  PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $PRPA_DIR, PRPA_SHARED_LIBADD)


#  PHP_CHECK_LIBRARY(,$LIBSYMBOL,
#  [
#     AC_DEFINE(HAVE_PRPALIB,1,[ ])
#  ],[
#    AC_MSG_ERROR([wrong rpa lib version or lib not found])
#  ],[
#    -LPRPA_DIR -lrpa -lrpasx -lm
#  ])

  PHP_SUBST(PRPA_SHARED_LIBADD)
  PHP_NEW_EXTENSION(prpa, prpa.c, $ext_shared)

fi
