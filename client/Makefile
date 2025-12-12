CLIENT_OUT = odb-cli
CFLAGS = -Wall -pedantic -I../inc/
LDFLAGS = -L../lib/ -lacip
CFILES = $(shell find . -name "*.c")
OFILES = $(CFILES:.c=.o)
CC = clang

.PHONY: all
all: $(OFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o ../$(CLIENT_OUT)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
