
LIB-OBJS := \
             syscall.o \
			 string.o \
			 determinism.o \
			 debug.o \
			 printfmt.o \
			 sprintf.o

-include $(patsubst %.o,lib/.deps/%.d,$(LIB-OBJS))

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

lib/lib.a: $(patsubst %.o,lib/%.o, $(LIB-OBJS))
	@echo "AR [$@]"
	$(V)ar rcs lib/lib.a $(patsubst %.o,lib/%.o, $(LIB-OBJS))

clean-lib:
	rm -rf lib/.deps lib/*.o lib/lib.a

LIB-TARGETS := lib/lib.a
ALL-TARGETS += $(LIB-TARGETS)

lib: $(LIB-TARGETS)

.PHONY += clean-lib

