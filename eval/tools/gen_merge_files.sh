#!/bin/bash

RAND=./rand
TMP=tmp

for i in 100 1000 10000 100000 1000000 10000000 50000000; do
	$RAND $i > $TMP/merge_$i
done

