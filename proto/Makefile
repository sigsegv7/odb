PROTOLIB_OUT = libacip.a
CFLAGS = -Wall -pedantic -I../inc/
CFILES = $(shell find . -name "*.c")
OFILES = $(CFILES:.c=.o)
CC = clang

.PHONY: all
all: $(OFILES)
	ar rcs ../lib/$(PROTOLIB_OUT) $^

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
