#!/bin/bash

if [ ! -e rand ]; then
	gcc -o rand rand.c
fi

./rand > ../merge_sort_arrays.h

