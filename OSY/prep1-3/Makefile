CC = gcc
CFLAGS = -Wall -Wextra -g -fsanitize=address,undefined -fno-omit-frame-pointer
LDFLAGS = -fsanitize=address,undefined

.PHONY: all clean

all: fileinfo followfile

fileinfo: fileinfo.c
	$(CC) $(CFLAGS) fileinfo.c -o fileinfo $(LDFLAGS)

followfile: followfile.c
	$(CC) $(CFLAGS) followfile.c -o followfile $(LDFLAGS)

clean:
	rm -f fileinfo followfile
	