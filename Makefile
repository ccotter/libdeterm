
DET-TARGETS := lib user

include $(patsubst %,%/Makefile,$(DET-TARGETS))

ARCH = x86_64
CFLAGS += -Wall -Wextra -Werror -nostdinc -nostdlib -g
ASFLAGS += -Wall -Wextra -Werror -nostdinc -nostdlib -g

V = 

CFLAGS += -Iinc -fno-builtin

clean: $(patsubst %,clean-%,$(DET-TARGETS))
	rm -rf

.PHONY := clean

