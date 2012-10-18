#!/bin/bash

function usage
{
	echo "Usage: $0 <num threads>"
	exit 1
}

# Generate 10 passwords and compare how long each algorithm runs.
# Three passwords length 2.
# Four passwords length 3.
# Three passwords length 3.

PASSWORDS="\
	g_ \
	T8 \
	e6 \
	38x \
	]9y \
	j;t \
	6xx \
	*\`Vg \
	Q=f5 \
	I}_s"

EXE=../user/exe/bcrack
NTHREADS=$1

if [ -z $NTHREADS ]; then
	usage
fi

for i in $PASSWORDS; do
	HASH=`echo -n $i | md5sum | cut -c -32`
	echo "time $EXE $NTHREADS $HASH"
	time $EXE $NTHREADS $HASH
	echo
done


