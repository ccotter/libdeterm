
USER-OBJS := \
	          six4.o \
			  fastsnap.o \
			  sync_test.o \
			  test.o \
			  merge.o \
			  fs_test.o \
			  regress.o \
			  zero.o \
			  grand.o \
			  regs.o \
			  copy.o \
			  fastfork.o \
			  bcrack.o

USER-EXES := $(patsubst %.o,user/exe/%, $(USER-OBJS))

-include $(patsubst %.o,user/.deps/%.d,$(USER-OBJS))

user/%.o: user/%.c
	@echo "CC [$<] > $@"
	$(V)mkdir -p user/.deps
	$(V)gcc -c $(CFLAGS) $< -o $@
	$(V)gcc -MP -MT $@ -MM $(CFLAGS) $< > $(patsubst user/%.c,user/.deps/%.d,$<)

user/exe/%: user/%.o lib/lib.a
	@echo EXE [$@]
	$(V)mkdir -p user/exe
	$(V)gcc $(CFLAGS) $^ -o $@

clean-user:
	rm -rf user/.deps user/*.o $(USER-EXES)

USER-TARGETS := $(USER-EXES)
ALL-TARGETS += $(USER-TARGETS)

user: $(USER-TARGETS)

.PHONY += clean-user

