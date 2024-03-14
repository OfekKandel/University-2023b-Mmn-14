CC = gcc
CFLAGS = -Wall -ansi -pedantic -g

main: main.c preprocessor.o parse_util.o macro_table.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm *.o
