#!/bin/bash

NAME=$1
DESTDIR=$2

if [ "$NAME" = "" ] || [ "$DESTDIR" = "" ]; then
    echo "$0 <testname> <destdir>"
    exit 1
fi

DESTFILE="$DESTDIR/$NAME"

echo "#!/bin/bash
if [ -f common.def ]; then
. common.def
fi

RPATEST=$NAME
DIFF=\"diff -u\"

if [ \"$RPAGREP\" = \"\" ]; then
RPAGREP=../rgrep/unix/x86_64/rgrep
fi


\$RPAGREP -p -f \$RPATEST.rpa -c \".*\" \$RPATEST.in > \$RPATEST.res
\$DIFF \$RPATEST.res \$RPATEST.out

if [ \$? != 0 ]; then
    echo \"\$RPATEST : FAILED\"
else
    echo \"\$RPATEST : PASSED\"
fi
" > $DESTFILE.sh

chmod ugo+x $DESTFILE.sh
if [ ! -f $DESTFILE.rpa ]; then
    echo "# RPA rules" > $DESTFILE.rpa
    echo "S			::= [#0x0009] | [#0x000B] | [#0x000C] | [#0x0020] | [#0x00A0] | [#0xFEFF]" >> $DESTFILE.rpa
fi

if [ ! -f $DESTFILE.in ]; then
    echo "sample text goes here" > $DESTFILE.in
fi
