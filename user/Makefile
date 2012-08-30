
USER-OBJS := \
	          six4.o

USER-EXES := $(patsubst %.o,user/exe/%, $(USER-OBJS))

-include $(patsubst %.o,user/.deps/%.d,$(USER-OBJS))

user/%.o: user/%.c
	@echo "CC [$<]"
	@mkdir -p user/.deps
	@gcc -c $(CFLAGS) $< -o $@
	@gcc -MP -MT $@ -MM $(CFLAGS) $< > $(patsubst user/%.c,user/.deps/%.d,$<)

user/exe/%: user/%.o lib/lib.a
	@echo EXE [$@]
	@mkdir -p user/exe
	@gcc $^ -o $@

clean-user:
	rm -rf user/.deps user/*.o $(USER-EXES)

.PHONY += clean-user

