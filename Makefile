CC       ?= cc
CFLAGS   ?= -Os -Wall -Wextra
LDFLAGS  += -Wl,-sectcreate,__DATA,cape,macos_15_cursors.cape -framework ApplicationServices -framework ImageIO -framework CoreFoundation
PREFIX   ?= /usr/local
AGENT    = $(HOME)/Library/LaunchAgents/local.retrocursor.plist
DOMAIN   = gui/$(shell id -u)

all: retrocursor

retrocursor: retrocursor.c macos_15_cursors.cape
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ retrocursor.c $(LDFLAGS)
	strip $@

run: retrocursor
	./retrocursor

install: retrocursor
	mkdir -p $(PREFIX)/bin
	install -m 755 retrocursor $(PREFIX)/bin/retrocursor

install-agent:
	mkdir -p $(dir $(AGENT))
	sed 's|@PREFIX@|$(PREFIX)|' local.retrocursor.plist.in > $(AGENT)
	@launchctl print $(DOMAIN)/local.retrocursor >/dev/null 2>&1 && launchctl bootout $(DOMAIN) $(AGENT) || true
	launchctl bootstrap $(DOMAIN) $(AGENT)

uninstall-agent:
	@launchctl print $(DOMAIN)/local.retrocursor >/dev/null 2>&1 && launchctl bootout $(DOMAIN) $(AGENT) || true
	rm -f $(AGENT)

clean:
	rm -f retrocursor

.PHONY: all run install install-agent uninstall-agent clean
