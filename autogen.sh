#!/bin/sh

# This script would only be necessary if your
# distribution is missing the 'configure'
# script.

##################################################
# Steps to product the "configure" shell program #
##################################################

# Creates macros needed by other programs
aclocal

autoconf

# produce the "config.h.in" file
autoheader


#################################################
# Create the "Makefile.in" using automake       #
# Assuming all "Makefile.am" files are in place #
#################################################
automake --add-missing


##################################################
# Generate the TAGS file (for emacs/vi) tags cmd #
##################################################
if [ ! -s src/TAGS ] ; then
 cd src
 etags *.[ch]
 cd ..
 echo "Tags file created in 'src' dir."
fi
