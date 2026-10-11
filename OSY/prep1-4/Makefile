CC = gcc
CFLAGS = -Wall -Wextra -g -fsanitize=address,undefined -fno-omit-frame-pointer
LDFLAGS = -fsanitize=address,undefined

.PHONY: all clean

all: pipeline

pipeline: pipeline.c
	$(CC) $(CFLAGS) pipeline.c -o pipeline $(LDFLAGS)

clean:
	rm -f pipeline
