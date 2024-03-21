CC = gcc
CFLAGS = -Wall -ansi -pedantic -g

main: main.c objs/preprocessor.o objs/parse_util.o objs/macro_table.o objs/assembler.o objs/symbol_table.o
	$(CC) $(CFLAGS) -o $@ $^

objs/%.o: %.c | objs
	$(CC) $(CFLAGS) -c $< -o $@

objs:
	mkdir -p objs

# Clean target
clean:
	rm -rf ./objs
