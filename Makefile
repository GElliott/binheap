CFLAGS := -m64 -O3 -march=native -std=gnu99
LDFLAGS := -L.
LDLIBS := -lbinheap -lrt

CC := gcc
LD := gcc
AR := ar

.PHONY: all

all: heaptest

clean:
	rm -f *.o *.a heaptest

libbinheap.a: binheap.c binheap.h sbinheap.c sbinheap.h defs.h
	$(CC) -c $(CFLAGS) binheap.c sbinheap.c
	$(AR) -r libbinheap.a binheap.o sbinheap.o

heaptest: main.c libbinheap.a
	$(CC) -c $(CFLAGS) main.c
	$(LD) $(LDFLAGS) main.o $(LDLIBS) -o heaptest
