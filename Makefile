
CC = gcc
AS = as
CC_FLAGS = -Wall -Wextra -Wno-unused -Wno-sign-compare -Wno-format -g
AS_FLAGS = -Wall -g

V = @

LIB_SRCS := lib/string.c \
			lib/determinism_pure.c \
			lib/mman.c \
			lib/syscall.c \
			lib/io.c \
			lib/std.c \
			lib/exec.c \
			lib/fork.c

LIB_ASM :=	lib/asm_syscall.S \
			lib/link.S

USER_SRCS :=	user/copyvm.c \
				user/copyvm2.c \
				user/copyvm2_snap.c \
				user/copyvm20.c \
				user/copyvm3.c \
				user/sixth.c \
				user/merge.c \
				user/qwe.c \
				user/Merge.c \
				user/proc.c \
				user/merge2.c \
				user/zerovm.c \
				user/zerovm2.c \
				user/zerovm3.c \
				user/test.c \
				user/parent.c \
				user/shell.c \
                user/ws.c \
                user/fs_demo.c

ULIB_SRCS :=	ulib/fork.c \
				ulib/determinism.c

PURE_SRCS :=	pure/det.c \
                pure/ws_worker.c \
				pure/merge3.c \
				pure/fs2.c \
				pure/fs3.c \
				pure/fs4.c \
				pure/ptest.c \
				pure/prog1.c \
				pure/prog2.c \
				pure/prog3.c \
				pure/prog4.c \
				pure/exec.c

FS_SRCS := fs/fs.c

LIB_OBJS = obj/lib/link.o obj/lib/asm_syscall.o
LIB_OBJS += $(patsubst lib/%.c, obj/lib/%.o, $(LIB_SRCS))
ULIB_OBJS = $(patsubst ulib/%.c, obj/ulib/%.o, $(ULIB_SRCS))
USER_OBJS = $(patsubst user/%.c, root/%, $(USER_SRCS))
PURE_OBJS = $(patsubst pure/%.c, root/%, $(PURE_SRCS))
FS_OBJS = $(patsubst fs/%.c, obj/fs/%.o, $(FS_SRCS))

INC_DEPS = $(wildcard inc/*.h)

obj/ulib/%.o: ulib/%.c
	@echo + cc[ULIB] $<
	$(V)$(CC) $(CC_FLAGS) -I. -c -o $@ $<

obj/ulib/link.o: ulib/link.S
	@echo + cc[ULIB] $<
	$(V)$(CC) $(CC_FLAGS) -I. -c -o $@ $<

root/%: user/%.c $(ULIB_OBJS) $(FS_OBJS) inc/determinism.h obj/ulib/link.o
	@echo + cc[USER] $<
	$(V)$(CC) $(CC_FLAGS) -static obj/ulib/link.o $(FS_OBJS) $(ULIB_OBJS) -I. -o $@ $<

obj/lib/fs_ext.o: lib/fs_ext.c
	$(V)$(CC) $(CC_FLAGS) -static -c -I. -o $@ $<

PURE_FLAGS = -nostdlib -nostdinc -nodefaultlibs -fno-builtin
obj/lib/%.o: lib/%.c
	@echo + cc[LIB] $<
	$(V)$(CC) $(CC_FLAGS) $(PURE_FLAGS) -I. -c -o $@ $<

obj/lib/%.o: lib/%.S
	@echo + as[LIB] $<
	$(V)$(CC) $(CC_FLAGS) $(PURE_FLAGS) -I. -c -o $@ $<

obj/fs/%.o: fs/%.c inc/fs.h
	@echo + cc[FS] $<
	$(V)$(CC) $(CC_FLAGS) $(PURE_FLAGS) -I. -c -o $@ $<

PURE_FLAGS = -nostdlib -nostdinc -nodefaultlibs -fno-builtin
root/%: pure/%.c $(LIB_OBJS) $(FS_OBJS)
	@echo + cc[PURE] $<
	$(V)$(CC) $(CC_FLAGS) $(PURE_FLAGS) -I. $(LIB_OBJS) $(FS_OBJS) -e _dstart -o $@ $<

lib: $(LIB_OBJS) $(ULIB_OBJS)
	@echo Made Library Objects

fs: $(FS_OBJS)
	@echo Made File System

pure: $(PURE_OBJS)
	@echo Made Pure Programs
#	./makeRoot.pl

rootfs: $(USER_OBJS)
	@echo + RootFS
	$(V)cat flist | cpio -o --format=newc > rootfs

user: $(USER_OBJS)
	@echo Made User Programs
#	./makeRoot.pl

clean:
	rm -rf obj/lib/* obj/fs/* root/* obj/pure/* obj/ulib/*

