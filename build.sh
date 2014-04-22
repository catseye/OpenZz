#!/bin/sh -x

cd src

SOURCES="
action
avl
defopen
dumpnet
err
interface
kernel
lazy
list
param
parse
printz
rule
scope
source
sys
table
util
zkernel
zlex
zsys
zz
zzi
"

LREADLINE=
DREADLINE=
if [ -z $WITH_READLINE ]; then
  cat >temp.c <<EOF
#include <readline/readline.h>
int main() { return 0; }
EOF
  if gcc temp.c -o temp; then
    WITH_READLINE=yes
  fi
fi
rm -f temp.c temp.o temp
if [ ! -z $WITH_READLINE ]; then
  LREADLINE='-lreadline'
  DREADLINE='-DUSE_READLINE'
fi

LDL=
if [ `uname -s` = 'Linux' ]; then
    LDL='-ldl'
fi

OBJECTS=""
for SOURCE in $SOURCES; do
  gcc -DVERSION='"1.0.4"' $DREADLINE -c $SOURCE.c -o $SOURCE.o || exit 1
  OBJECTS="$OBJECTS $SOURCE.o"
done

gcc $OBJECTS -o ozz $LDL $LREADLINE

cd ..
