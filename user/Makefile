
USER-OBJS := \
	          six4.o

#-include $(USER-OBJS:.o=.d)
-include $(patsubst %.o,user/.deps/%.d,$(USER-OBJS))

user/%.o: user/%.c
	gcc -c $(CFLAGS) $< -o $@
	gcc -MP -MT $@ -MM $(CFLAGS) $< > $(patsubst user/%.c,user/.deps/%.d,$<)

