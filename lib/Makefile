
.PHONY += lib

obj/lib/syscall.o: inc/*.h
	$(CC) $(CFLAGS) -c lib/$(ARCH)/syscall.S -o obj/lib/syscall.o

obj/lib/string.o: inc/*.h
	$(CC) $(CFLAGS) -c lib/string.c -o obj/lib/string.o

lib: obj/lib/syscall.o obj/lib/string.o
	ar rcs obj/lib.a obj/lib/syscall.o obj/lib/string.o

