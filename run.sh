#! /bin/sh
#
# Workaround.  CLion can't redirect input (what?!) so I will use a shell
# script to pipe input to the parser.
#
echo "Running"
cat $2 | $1
echo "Done"
