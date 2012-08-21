
.PHONY = obj/lib lib clean

include lib/Makefile

ARCH = x86_64
CC = gcc
AS = gcc
CFLAGS += -Wall -Wextra -Werror -nostdinc -nostdlib -g
ASFLAGS += -Wall -Wextra -Werror -nostdinc -nostdlib -g

V = 

CFLAGS += -Iinc -fno-builtin

six4: lib user/six4.c
	$(V)$(CC) $(CFLAGS) -static user/six4.c obj/lib.a -o six4

clean:
	rm -rf obj/lib/* six4

