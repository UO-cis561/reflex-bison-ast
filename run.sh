#! /bin/sh
#
# Workaround.  CLion can't redirect input (what?!) so I will use a shell
# script to pipe input to the parser.
#
echo "dollar star is '$*'"
args="$*"
bin/parser  $args
echo "Did it"
exit 0
