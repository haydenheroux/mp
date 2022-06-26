include config.mk

SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)

all: options mp

options:
	@echo mp build options:
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo "CC      = $(CC)"

.c.o:
	$(CC) -c $(CFLAGS) $<

mp: $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

clean:
	rm -f mp $(OBJ)

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f mp $(DESTDIR)$(PREFIX)/bin
	chmod 775 $(DESTDIR)$(PREFIX)/bin/mp

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/mp

.PHONY: all options clean install uninstall
