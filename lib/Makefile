
CORE-LIB-OBJS := \
             syscall.o \
			 string.o \
			 determinism.o \
			 determinism_asm.o \
			 debug.o \
			 printfmt.o \
			 sprintf.o \
			 entry.o \
			 stdlib.o \
			 syscalls.o \
			 mman.o \
			 bench.o \
			 time.o

FS-LIB-OBJS := \
		   fs.o

LIB-OBJS := $(patsubst %.o,lib/%.o,$(CORE-LIB-OBJS)) \
		$(patsubst %.o,lib/fs/%.o,$(FS-LIB-OBJS))

-include $(patsubst %.o,lib/.deps/%.d,$(CORE-LIB-OBJS))
-include $(patsubst %.o,lib/fs/.deps/%.d,$(FS-LIB-OBJS))

lib/fs/%.o: lib/fs/%.c
	@echo "CC [$<] > $@"
	$(V)mkdir -p lib/fs/.deps
	$(V)gcc -c $(CFLAGS) $< -o $@
	$(V)gcc -MP -MT $@ -MM $(CFLAGS) $< > $(patsubst lib/fs/%.c,lib/fs/.deps/%.d,$<)

lib/%.o: lib/%.c
	@echo "CC [$<] > $@"
	$(V)mkdir -p lib/.deps
	$(V)gcc -c $(CFLAGS) $< -o $@
	$(V)gcc -MP -MT $@ -MM $(CFLAGS) $< > $(patsubst lib/%.c,lib/.deps/%.d,$<)

lib/%.o: lib/%.S
	@echo "AS [$<] > $@"
	$(V)mkdir -p lib/.deps
	$(V)gcc -c $(CFLAGS) $< -o $@
	$(V)gcc -MP -MT $@ -MM $(CFLAGS) $< > $(patsubst lib/%.S,lib/.deps/%.d,$<)

lib/lib.a: $(LIB-OBJS)
	@echo "AR [$@]"
	$(V)ar rcs lib/lib.a $(LIB-OBJS)

clean-lib:
	rm -rf lib/.deps lib/*.o lib/lib.a

LIB-TARGETS := lib/lib.a
ALL-TARGETS += $(LIB-TARGETS)

lib: $(LIB-TARGETS)

.PHONY += clean-lib

