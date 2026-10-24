CC     = gcc
CFLAGS = -Wall -MMD -MP
OBJS   = main.o

prog: $(OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

clean:
	rm -f prog $(OBJS) $(OBJS:.o=.d)

.PHONY: clean
