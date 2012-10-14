#!/bin/bash

function usage()
{
	echo "Usage: $0 <type>"
	echo "  <type>: s for serial, p for parallel"
	exit 1
}

EXE=exe/merge

TYPE=$1

if [ "$TYPE" != "s" ] && [ "$TYPE" != "p" ]; then
	usage
fi

for i in 100 1000 10000 100000 1000000 10000000 50000000; do
	echo -n "time $EXE -d 5 -t $TYPE -f tmp/merge_$i"
	time $EXE -d 5 -t $TYPE -f tmp/merge_$i
	if [ $? != 0 ]; then
		echo "Sorting failure!"
		exit 1;
	fi
	echo
done

