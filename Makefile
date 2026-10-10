CC       ?= cc
CFLAGS   ?= -Os -Wall -Wextra
LDFLAGS  += -framework ApplicationServices -framework ImageIO -framework CoreFoundation
PREFIX   ?= /usr/local

all: retrocursor

retrocursor: retrocursor.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ retrocursor.c $(LDFLAGS)
	strip $@

run: retrocursor
	./retrocursor

install: retrocursor
	install -d $(PREFIX)/bin
	install -m 755 retrocursor $(PREFIX)/bin/retrocursor

clean:
	rm -f retrocursor

.PHONY: all run install clean
