CC = gcc
CFLAGS = -Wall -Wextra

todo: todo.c
	$(CC) $(CFLAGS) -o todo todo.c -lncurses

clean:
	rm -f todo

.PHONY: clean