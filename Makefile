CC       ?= cc
CFLAGS   ?= -Os -Wall -Wextra
LDFLAGS  += -framework ApplicationServices -framework ImageIO -framework CoreFoundation
PREFIX   ?= /usr/local
AGENT    = $(HOME)/Library/LaunchAgents/local.retrocursor.plist
DOMAIN   = gui/$(shell id -u)

all: retrocursor

retrocursor: retrocursor.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ retrocursor.c $(LDFLAGS)
	strip $@

run: retrocursor
	./retrocursor

install: retrocursor
	install -d $(PREFIX)/bin
	install -m 755 retrocursor $(PREFIX)/bin/retrocursor

install-agent:
	mkdir -p $(dir $(AGENT))
	sed 's|@PREFIX@|$(PREFIX)|' local.retrocursor.plist.in > $(AGENT)
	-launchctl bootout $(DOMAIN) $(AGENT) 2>/dev/null
	launchctl bootstrap $(DOMAIN) $(AGENT)

uninstall-agent:
	-launchctl bootout $(DOMAIN) $(AGENT)
	rm -f $(AGENT)

clean:
	rm -f retrocursor

.PHONY: all run install install-agent uninstall-agent clean
