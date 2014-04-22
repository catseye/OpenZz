#!/bin/sh

# Zz Test Script
# James Brooks, Feb/2002
# Run this script to run Zz against all ".zz" files in the current directory
# and compare the output against the expected output.  This output should be 
# defined in the commented header of each file.
# Some processing symbol(s) are defined:
#  IGNORE_LINE_PATTERN=<regexp>  - identifies lines to ignore in comparison


CURRENT_RESULT=""
FAILURE_LIST=""
FAILURE_COUNT=0
TOTAL_COUNT=0
TESTFILE_PATTERN="*.zz"
REMOVE_TMP_FILES=0
TESTDIR="./"
TEMP_DIFFFILE="/tmp/zztest.diff"
ZZ=ozz
#IGNORE_LINE_PATTERN='(^\| +line|^listed)'

# Regular expression for 'sed' to begin extracting test results from files.
REF_RANGE="/[Rr][Ee][Ff][Ee][Rr][Ee][Nn][Cc][Ee].[Oo][Uu][Tt]/,/[Ee][Nn][Dd].[Oo][Uu][Tt]/p"


# Safety check, make sure zz is in path
which $ZZ  >/dev/null 2>&1
if [ $? -ne 0 ];
then
  echo "Could not locate \"$ZZ\" program for testing, not in path."
  echo "Exiting."
  exit 1
fi


# Brief help output
showusage()
{
  echo "Usage: $0 [-h] [-d <path>] [<testfile>]"
  echo "  -h print short help"
  echo "  -d run test on .zz files in <path>"
  echo "  <testfilr>  run test script against only specified file and"
  echo "              leave tmp files around for inspection."
}


# Process command line args
args=`getopt "hd:" $*`

if [ $? -ne 0 ];
then
  showusage
  exit 1
else
  set -- $args

  while [ "$1" != "" ] ; do
    opt="$1" ; shift
    case "$opt" in
      -d)
	  TESTDIR="$1"
	  shift
	  ;;
      -h)
          showusage;
          exit 0;;
      --)
          #shift;    ???
          break;;
    esac
  done
fi

if [ "$1" ];
then
  if [ "$2" ];
  then
    echo " unknown usage: too many parameters: $*"
    showusage
    exit 1;
  fi

  TESTFILE_PATTERN="$1";
  REMOVE_TMP_FILES=0
fi


# Function for padding output for columns
paddit()
{
  TEMP=$1
  LEN=`echo "$TEMP" | wc -L`

  while [ $LEN -le 40 ]; 
  do
    TEMP=$TEMP"."
    LEN=`expr $LEN + 1`
  done

 echo $TEMP$2
}


compare_temp_files()
{
  F1=$1
  F2=$2
  # Test for processing instructions in source file:
  # NOTE: there can be at most one!
  #PROC_CMD_STR="`grep "IGNORE_LINE_PATTERN" $F1 | cut -d= -f2`"
#  set -x 
  if [ -n "$IGNORE_LINE_PATTERN" ]; then
#    diff --ignore-matching-lines="$PROC_CMD_STR" $F1 $F2 >$TEMP_DIFFFILE
    diff --ignore-matching-lines="$IGNORE_LINE_PATTERN" $F1  $F2 >$TEMP_DIFFFILE
  else
    diff $F1 $F2 >$TEMP_DIFFFILE
#    cmp -s $TEMP_FILE1 $TEMP_FILE2
  fi

  return $?;

#  set +x
}


echo
echo "Testing OZz against $TESTFILE_PATTERN testfile(s) in current directory:"
echo


for TEST_FILE in $TESTDIR/$TESTFILE_PATTERN; do

  BASE_TEST_FILE=`basename $TEST_FILE`
  CURRENT_RESULT="Testing File: "$BASE_TEST_FILE

  TEMP_FILE1="/tmp/zzexpected.tmp"
  TEMP_FILE2="/tmp/zzoutput.tmp"

  # Parse out the "Reference Output" from the source scripts
  grep "^\!\!" $TEST_FILE | sed -n $REF_RANGE | sed '1d' | sed '$d' | sed 's/[ ]*$//' | cut -c3- > $TEMP_FILE1

  $ZZ $TEST_FILE 2>&1 | sed 's/[ ]*$//' > $TEMP_FILE2

  # Performs managed comparison on output temp files:
  # Returns 0 for success, 1 for failure(a difference)
  compare_temp_files $TEMP_FILE1  $TEMP_FILE2

  # If an error occured remember the filename and count of failures for reporting:
  if [ $? -eq 0 ];
  then
    paddit "$CURRENT_RESULT" "OK"
  else
    FAILURE_LIST=$FAILURE_LIST" "$TEST_FILE
    FAILURE_COUNT=`expr $FAILURE_COUNT + 1`

    paddit "$CURRENT_RESULT" "FAILED"
    mv $TEMP_DIFFFILE "$BASE_TEST_FILE.diff"
  fi

  #Increment total count - number of files tested
  TOTAL_COUNT=`expr $TOTAL_COUNT + 1`

  #Cleanup
  if [ $REMOVE_TMP_FILES -ne 0 ]; then
    [ -e $TEMP_FILE1 ] && rm $TEMP_FILE1
    [ -e $TEMP_FILE2 ] && rm $TEMP_FILE2
  fi

done


echo
echo "Finished Test Scripts: "$TOTAL_COUNT" test scripts executed."
echo

if [ $FAILURE_COUNT -ne 0 ];
then
  echo "ERRORS: The following "$FAILURE_COUNT" file(s) produced output other than expected during tests:"
  echo $FAILURE_LIST
  echo
  exit 1
else
  echo "SUCCESS! All tests successful."
  echo
fi
