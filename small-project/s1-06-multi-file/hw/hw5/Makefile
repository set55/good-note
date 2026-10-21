CC = gcc
CFLAGS = -Wall -g -O0 -MMD -MP
OBJS = logger.o worker.o hw2_main.o

logdemo: $(OBJS)
	$(CC) $(CFLAGS) -o logdemo $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

clean:
	rm -f $(OBJS) logdemo *.d

.PHONY: clean

