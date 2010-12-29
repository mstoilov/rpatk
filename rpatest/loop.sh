#!/bin/bash
if [ -f common.def ]; then
. common.def
fi

RPATEST=loop
DIFF="diff -u"

if [ "" = "" ]; then
RPAGREP=../rgrep/unix/x86_64/rgrep
fi


$RPAGREP -p -f $RPATEST.rpa -c ".*" $RPATEST.in > $RPATEST.res
$DIFF $RPATEST.res $RPATEST.out

if [ $? != 0 ]; then
    echo "$RPATEST : FAILED"
else
    echo "$RPATEST : PASSED"
fi

