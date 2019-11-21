#! /bin/sh
#
# Workaround.  CLion can't redirect input (what?!) so I will use a shell
# script to pipe input to the parser.
#
compiler="$1"
source="$2"
echo "Before shift, args are $*"
shift ; shift
echo "After shift, args are $*"
echo "Running"
cat $source | $compiler $*
echo "Done"
