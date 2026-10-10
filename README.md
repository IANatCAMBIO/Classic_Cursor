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

Easiest: **System Settings → General → Login Items → +**, then pick
`/usr/local/bin/retrocursor`. (macOS may open a Terminal window for a bare
executable.) For a silent run, use a LaunchAgent at
`~/Library/LaunchAgents/local.retrocursor.plist` with `ProgramArguments`
`/usr/local/bin/retrocursor` and `RunAtLoad` set to true.

## How it works

Parses the cape plist, decodes each cursor's images, and registers them with
WindowServer through the private CoreGraphics cursor API. The Arrow and I-beam
cursors are also registered under every system cursor name containing
"arrow"/"ibeam", since macOS 26 renamed them.

## Credits

The approach and the private API usage come from
[Mousecape](https://github.com/alexzielenski/Mousecape) by Alex Zielenski and
the Tahoe-support fork this was derived from. No Mousecape source is included.
