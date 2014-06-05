CC = gcc
CFLAGS = -Wall -pedantic
LDFLAGS = -lm

.PHONY: all
all: txt2bin

txt2bin: txt2bin.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

.PHONY: install
install: txt2bin
	install -c txt2bin /usr/local/bin

.PHONY: clean
clean:
	rm -f txt2bin
