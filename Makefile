CC      := gcc
CFLAGS  := -Wall -Wextra -fPIC -O2
LDFLAGS := -shared
LIBS    := -pthread

.PHONY: all clean

all: libhashmap.so test_hashmap

libhashmap.so: hashmap.o
	$(CC) $(LDFLAGS) -o $@ $^

hashmap.o: hashmap.c hashmap.h
	$(CC) $(CFLAGS) -c $<

test_hashmap.o: test_hashmap.c hashmap.h
	$(CC) $(CFLAGS) -c $<

# rpath '$$ORIGIN' ensures the exe finds libhashmap.so in the same dir
test_hashmap: test_hashmap.o libhashmap.so
	$(CC) -o $@ test_hashmap.o -L. -Wl,-rpath,'$$ORIGIN' -lhashmap $(LIBS)

clean:
	rm -f *.o libhashmap.so test_hashmap
