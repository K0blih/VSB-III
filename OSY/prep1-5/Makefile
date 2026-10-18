CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g

# make sestavi vsechny programy. Napriklad make gen sestavi jen gen.
all: calcpipe gen filter transform sum io_practice

# Za dvojteckou jsou soubory, pri jejichz zmene se program znovu sestavi.
# Prikaz kompilatoru musi zacinat tabulatorem.
calcpipe: calcpipe.c common.h
	$(CC) $(CFLAGS) calcpipe.c -o calcpipe

gen: gen.c
	$(CC) $(CFLAGS) gen.c -o gen

filter: filter.c
	$(CC) $(CFLAGS) filter.c -o filter

# -lm pripoji matematickou knihovnu pro sqrt().
transform: transform.c
	$(CC) $(CFLAGS) transform.c -o transform -lm

sum: sum.c
	$(CC) $(CFLAGS) sum.c -o sum

io_practice: io_practice.c
	$(CC) $(CFLAGS) io_practice.c -o io_practice

# make clean odstrani sestavene programy, zdrojove soubory zustanou.
clean:
	rm -f calcpipe
	rm -f gen
	rm -f filter
	rm -f transform
	rm -f sum
	rm -f io_practice

# all a clean jsou nazvy akci, nikoli souboru.
.PHONY: all clean
