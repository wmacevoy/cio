CSTD?=-std=c11
CDBG?=-g
COPT?=
CINC?=-Iinclude -I../facts/include -I../utf8/include

CFLAGS=$(CDBG) $(COPT) $(CSTD) $(CINC)

LDLIBS=-lm

.PHONY: all
all : bin/cio_facts

.PHONY: check
check : all
	bin/cio_facts | diff - expected/cio_facts.out

.PHONY: expected
expected : all
	bin/cio_facts >expected/cio_facts.out || true

tmp/facts.o: ../facts/src/facts.c ../facts/include/facts.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

tmp/utf8.o: ../utf8/src/utf8.c ../utf8/include/utf8.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

tmp/cio.o: src/cio.c include/cio.h ../utf8/include/utf8.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

tmp/cio_facts.o : src/cio_facts.c include/cio.h ../utf8/include/utf8.h ../facts/include/facts.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

bin/cio_facts : tmp/cio_facts.o tmp/utf8.o tmp/cio.o tmp/facts.o
	mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
