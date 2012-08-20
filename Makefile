
.PHONY = obj/lib lib clean

include lib/Makefile.inc

ARCH = x86_64
CC = gcc
AS = gcc
CFLAGS += -Wall -Wextra -Werror
ASFLAGS += -Wall -g

V = 

CFLAGS += -Iinc

six4: lib user/six4.c
	$(V)$(CC) $(CFLAGS) -static user/six4.c obj/lib/syscall.o -o six4

clean:
	rm -rf syscall_64.o six4

