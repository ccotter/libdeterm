
LIB-OBJS := \
             syscall.o \
			 string.o

-include $(patsubst %.o,lib/.deps/%.d,$(LIB-OBJS))

lib/%.o: lib/%.c
	@echo "CC [$<]"
	@mkdir -p lib/.deps
	@gcc -c $(CFLAGS) $< -o $@
	@gcc -MP -MT $@ -MM $(CFLAGS) $< > $(patsubst lib/%.c,lib/.deps/%.d,$<)

lib/%.o: lib/%.S
	@echo "AS [$<]"
	@mkdir -p lib/.deps
	@gcc -c $(CFLAGS) $< -o $@
	@gcc -MP -MT $@ -MM $(CFLAGS) $< > $(patsubst lib/%.S,lib/.deps/%.d,$<)

lib/lib.a: $(patsubst %.o,lib/%.o, $(LIB-OBJS))
	@echo "AR [$@]"
	@ar rcs lib/lib.a $(patsubst %.o,lib/%.o, $(LIB-OBJS))

clean-lib:
	rm -rf lib/.deps lib/*.o lib/lib.a

.PHONY += clean-lib

