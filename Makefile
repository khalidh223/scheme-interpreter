SRCS = linkedlist.c talloc.c main.c tokenizer.c parser.c interpreter.c
HDRS = headers/tokenizer.h headers/linkedlist.h headers/talloc.h headers/parser.h headers/value.h headers/interpreter.h

CC = clang
CFLAGS = -g

OBJS = $(SRCS:.c=.o)

.PHONY: interpreter
interpreter: $(OBJS)
	$(CC)  $(CFLAGS) $^  -o $@
	rm -f *.o
	rm -f vgcore.*

.PHONY: phony_target
phony_target:

%.o : %.c $(HDRS) phony_target
	$(CC)  $(CFLAGS) -c $<  -o $@

clean:
	rm -f *.o
	rm -f interpreter

