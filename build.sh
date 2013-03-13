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

OBJECTS=""
for SOURCE in $SOURCES; do
  gcc -DVERSION='"1.0.4"' -c $SOURCE.c -o $SOURCE.o || exit 1
  OBJECTS="$OBJECTS $SOURCE.o"
done

gcc $OBJECTS -o ozz -ldl -lreadline

cd ..
