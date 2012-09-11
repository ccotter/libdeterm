
ALL-TARGETS :=
DET-TARGETS := lib user

# Help menu.
help:
	@echo "Libdeterm C user library build system."
	@echo "  all: Build all targets."
	@echo "  lib: Build user library."
	@echo "  user: Build executables under user/."

include $(patsubst %,%/Makefile,$(DET-TARGETS))

# Some global macros.
ARCH = x86_64
CFLAGS += -Wall -Wextra -Werror -nostdinc -nostdlib -g -Wno-unused -std=gnu99
ASFLAGS += -Wall -Wextra -Werror -nostdinc -nostdlib -g

V = @

CFLAGS += -Iinc -fno-builtin

# Build targets.
all: $(ALL-TARGETS)

# Clean up, etc.
CLEAN-TARGETS := $(patsubst %,clean-%,$(DET-TARGETS))
clean: $(CLEAN-TARGETS)

.PHONY := all user lib clean $(CLEAN-TARGETS)

