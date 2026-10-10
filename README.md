# RetroCursor

A tiny one-shot tool that replaces the macOS cursors with the ones in a
[Mousecape](https://github.com/alexzielenski/Mousecape) `.cape` file, then exits.

By default it applies `/Users/ian_campbell/Downloads/local.mousecape.macos.15.basics.cape`.
Pass a different path as the first argument to use another cape.

Cursors stay replaced until logout or reboot. There is no backup/restore; run it
again at each login (see below).

## Install

Prerequisites: macOS with Xcode Command Line Tools (`xcode-select --install`).

```sh
make              # build ./retrocursor (~35 KB)
make run          # build and apply the cape now
sudo make install # copies to /usr/local/bin/retrocursor (PREFIX=... to change)
```

To bake in a different default cape:
`make CPPFLAGS='-DDEFAULT_CAPE=\"/path/to/file.cape\"'`

## Run at login

```sh
sudo make install        # binary to /usr/local/bin
make install-agent       # installs and loads a LaunchAgent (silent, runs at every login)
make uninstall-agent     # removes it
```

`install-agent` writes `~/Library/LaunchAgents/local.retrocursor.plist` from
`local.retrocursor.plist.in` and loads it with `launchctl`. It also appears in
System Settings → General → Login Items & Extensions → Allow in the Background.
Run it without `sudo` so the agent lands in your own home folder (if `/usr/local/bin`
needs root, run `sudo make install` first, then `make install-agent`).

## How it works

Parses the cape plist, decodes each cursor's images, and registers them with
WindowServer through the private CoreGraphics cursor API. The Arrow and I-beam
cursors are also registered under every system cursor name containing
"arrow"/"ibeam", since macOS 26 renamed them.

## Credits

The approach and the private API usage come from
[Mousecape](https://github.com/alexzielenski/Mousecape) by Alex Zielenski and
the Tahoe-support fork this was derived from. No Mousecape source is included.
