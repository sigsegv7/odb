ACI_OUT = odb.d
CFLAGS = -Wall -pedantic -I../inc/
CFILES = $(shell find . -name "*.c")
OFILES = $(CFILES:.c=.o)
CC = clang

.PHONY: all
all: $(OFILES)
	$(CC) $(CFLAGS) $^ -o ../$(ACI_OUT)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
