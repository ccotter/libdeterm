
.PHONY = obj/lib lib clean

CC = gcc
AS = gcc
CFLAGS += -Wall -Wextra -Werror
ASFLAGS += -Wall -g

V = 

CFLAGS += -Iinc

six4:
	$(V)$(AS) $(ASFLAGS) -c lib/syscall_64.S -o syscall_64.o
	$(V)$(CC) $(CFLAGS) user/six4.c syscall_64.o -o six4

clean:
	rm -rf syscall_64.o six4

