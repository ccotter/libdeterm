#!/usr/bin/python

import sys
import random

def usage():
	print("Usage:", sys.argv[0], "<size>")
	exit(1)

if len(sys.argv) != 2:
	usage()

l = int(sys.argv[1])
print(l, end=' ')
for i in range(0, l):
	print(random.randrange(0, 10000000), end=' ')

