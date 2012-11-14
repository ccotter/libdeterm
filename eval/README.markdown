
Instructions and other notes for evaluations.

Merge sort
~~~~~~~~~~

This evaluation sorts a list of integers using serial or parallel merge sort.
The integers are hardcoded into the executable. The benchmark sorts each array
two times back-to-back and times the second sort.
Run eval/tools/gen_merge_files to create files with random arrays to sort. These
files go into eval/tmp.

Legacy Linux
------------

Program exists as eval/merge.c, executable in eval/exe/merge.
Use eval_merge.sh to run serial and parallel versions.
./eval_merge.sh s > results/merge_serial 2>&1
./eval_merge.sh p > results/merge_parallel 2>&1

Deterministic Linux
-------------------
Program code exist as user/merge_sort.c, executable in user/exe/merge_sort.
TODO

MD5 brute force password cracker
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TODO

Matrix Multiplication
~~~~~~~~~~~~~~~~~~~~~

TODO

