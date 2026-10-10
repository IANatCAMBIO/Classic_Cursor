CC       ?= cc
CFLAGS   ?= -Os -Wall -Wextra
LDFLAGS  += -Wl,-sectcreate,__DATA,cape,macos_15_cursors.cape -framework ApplicationServices -framework ImageIO -framework CoreFoundation
PREFIX   ?= /usr/local
AGENT    = $(HOME)/Library/LaunchAgents/local.classic_cursor.plist
DOMAIN   = gui/$(shell id -u)

all: classic_cursor

classic_cursor: classic_cursor.c macos_15_cursors.cape
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ classic_cursor.c $(LDFLAGS)
	strip $@

run: classic_cursor
	./classic_cursor

install: classic_cursor
	mkdir -p $(PREFIX)/bin
	install -m 755 classic_cursor $(PREFIX)/bin/classic_cursor

install-agent:
	mkdir -p $(dir $(AGENT))
	sed 's|@PREFIX@|$(PREFIX)|' local.classic_cursor.plist.in > $(AGENT)
	@launchctl print $(DOMAIN)/local.classic_cursor >/dev/null 2>&1 && launchctl bootout $(DOMAIN) $(AGENT) || true
	launchctl bootstrap $(DOMAIN) $(AGENT)

uninstall-agent:
	@launchctl print $(DOMAIN)/local.classic_cursor >/dev/null 2>&1 && launchctl bootout $(DOMAIN) $(AGENT) || true
	rm -f $(AGENT)

clean:
	rm -f classic_cursor

.PHONY: all run install install-agent uninstall-agent clean
