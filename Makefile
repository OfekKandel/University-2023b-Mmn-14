CC = gcc
CFLAGS = -Wall -ansi -pedantic -g

main: main.c objs/preprocessor.o objs/parse_util.o objs/macro_table.o
	$(CC) $(CFLAGS) -o $@ $^

objs/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

objs:
	mkdir objs

# Clean target
clean:
	rm -rf ./objs
