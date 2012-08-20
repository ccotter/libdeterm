
.PHONY += lib

obj/lib/syscall.o: inc/syscall.h inc/syscall_32.h inc/syscall_64.h
	$(CC) $(CCFLAGS) -c lib/$(ARCH)/syscall.S -o obj/lib/syscall.o

lib: obj/lib/syscall.o

