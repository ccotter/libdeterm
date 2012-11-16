
#Deterministic C Library for Linux

 * Deterministic syscall interface
 * Emulate Linux C standard library functions (eg. mmap, fork)
 * Example programs

#Current code

The current project branch is being redesigned to build 32 and 64 bit versions
of the library. As such, the status of this branch is purely development and
has a lot of test code/prototypes.

##Acknowledgements

Some of the code in `inc/` and `lib/` is derived or copied exactly from MIT's
JOS operating system. I will do my best to document which files came from JOS.
See NOTICES for JOS's license.

### Determinator
The library itself is in many ways inspired by
[Determinator](https://github.com/bford/Determinator). See ["Efficient
System-Enforced Deterministic Parallelism](
http://static.usenix.org/event/osdi10/tech/full_papers/Aviram.pdf).

