
EVAL-OBJS := \
			  pqsort.o \
			  lu.o \
			  lu_cont.o \
			  bcrack.o \
			  matmult.o \
			  merge.o

EVAL-EXES := $(patsubst %.o,eval/exe/%, $(EVAL-OBJS))

-include $(patsubst %.o,eval/.deps/%.d,$(EVAL-OBJS))

CFLAGS-NONDET := -Wall -Wextra -Werror -g -Wno-unused -std=gnu99 -lpthread

eval/%.o: eval/%.c
	@echo "CC [$<] > $@"
	$(V)mkdir -p eval/.deps
	$(V)gcc -c $(CFLAGS-NONDET) $< -o $@
	$(V)gcc -MP -MT $@ -MM $(CFLAGS-NONDET) $< > $(patsubst eval/%.c,eval/.deps/%.d,$<)

eval/exe/%: eval/%.o
	@echo EXE [$@]
	$(V)mkdir -p eval/exe
	$(V)gcc $(CFLAGS-NONDET) $^ -o $@

clean-eval:
	rm -rf eval/.deps eval/*.o $(EVAL-EXES)

EVAL-TARGETS := $(EVAL-EXES)
ALL-TARGETS += $(EVAL-TARGETS)

eval: $(EVAL-TARGETS)

.PHONY += clean-eval

