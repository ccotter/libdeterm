
.PHONY = obj/lib lib clean

include lib/Makefile
include user/Makefile

ARCH = x86_64
CC = gcc
AS = gcc
CFLAGS += -Wall -Wextra -Werror -nostdinc -nostdlib -g
ASFLAGS += -Wall -Wextra -Werror -nostdinc -nostdlib -g

V = 

CFLAGS += -Iinc -fno-builtin

clean:
	rm -rf

