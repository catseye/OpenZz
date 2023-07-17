#!/bin/sh -ex

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
cat >temp.c <<EOF
#include <readline/readline.h>
int main() { return 0; }
EOF
if gcc temp.c -o temp; then
  DREADLINE='-DUSE_READLINE'
  LREADLINE='-lreadline'
  if gcc temp.c -o temp -ledit; then
    LREADLINE='-ledit'
  fi
fi
rm -f temp.c temp.o temp
if [ ! -z $WITHOUT_READLINE ]; then
  LREADLINE=
  DREADLINE=
fi

LDL=
if [ `uname -s` = 'Linux' ]; then
    LDL='-ldl'
fi

# ozz doesn't work as a 64-bit executable, so we build it as 32-bit.
# Note, this requires 32-bit headers and libraries be present on the system.
CFLAGS=-m32

OBJECTS=""
for SOURCE in $SOURCES; do
  gcc $CFLAGS -DVERSION='"1.0.4-4ce3"' $DREADLINE -c $SOURCE.c -o $SOURCE.o || exit 1
  OBJECTS="$OBJECTS $SOURCE.o"
done

gcc $CFLAGS $OBJECTS -o ozz $LDL $LREADLINE

cd ..
