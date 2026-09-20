CC = gcc
AR = ar
CPPFLAGS = -I.
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2

.PHONY: all clean

all: genstr strcheck small_lib/libstrcheck.so big_lib/libstrcheck.so

genstr: genstr.o libgenstr.a
	$(CC) $(LDFLAGS) -o $@ genstr.o libgenstr.a $(LDLIBS)

libgenstr.a: genstr_lib.o
	$(AR) rcs $@ $^

genstr.o: genstr.c genstr_lib.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

genstr_lib.o: genstr_lib.c genstr_lib.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

strcheck.o: strcheck.c strcheck_lib.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

small_lib/strcheck_lib.o: small_lib/strcheck_lib.c strcheck_lib.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -c $< -o $@

big_lib/strcheck_lib.o: big_lib/strcheck_lib.c strcheck_lib.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -c $< -o $@

small_lib/libstrcheck.so: small_lib/strcheck_lib.o
	$(CC) $(LDFLAGS) -shared -Wl,-soname,libstrcheck.so -o $@ $^ $(LDLIBS)

big_lib/libstrcheck.so: big_lib/strcheck_lib.o
	$(CC) $(LDFLAGS) -shared -Wl,-soname,libstrcheck.so -o $@ $^ $(LDLIBS)

strcheck: strcheck.o small_lib/libstrcheck.so
	$(CC) $(LDFLAGS) -o $@ strcheck.o -Lsmall_lib -lstrcheck $(LDLIBS)

clean:
	rm -f genstr strcheck *.o libgenstr.a small_lib/*.o small_lib/*.so big_lib/*.o big_lib/*.so
