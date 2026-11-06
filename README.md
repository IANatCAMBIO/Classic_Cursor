# classic_cursor

A tiny one-shot tool that replaces the macOS cursors with the ones in a
[Mousecape](https://github.com/alexzielenski/Mousecape) `.cape` file, then exits.

The cape (`macos_15_cursors.cape` in this repo) is embedded in the binary at build time, so
the installed binary has no runtime file dependency. To use a different cape,
replace `macos_15_cursors.cape` and rebuild.

Cursors stay replaced until logout or reboot. There is no backup/restore; run it
again at each login (see below).

## Install

Prerequisites: macOS with Xcode Command Line Tools (`xcode-select --install`) and an
Apple Development signing certificate in your login keychain (see BUILD.md, Signing).

```sh
make              # build and sign ./classic_cursor (cape included); run as your user
make run          # build and apply the cape now
make install      # copies to ~/.local/bin/classic_cursor (PREFIX=... to change)
```

## Run at login

```sh
make install             # binary to ~/.local/bin (no sudo needed)
make install-agent       # installs and loads a LaunchAgent (silent, runs at every login)
make uninstall-agent     # removes it
```

`install-agent` writes `~/Library/LaunchAgents/local.classic_cursor.plist` from
`local.classic_cursor.plist.in` and loads it with `launchctl`. It also appears in
System Settings → General → Login Items & Extensions → Allow in the Background.
`~/.local/bin` is not on PATH by default on macOS; add it to your shell profile if you
want to run `classic_cursor` by hand. The LaunchAgent uses the full path, so it works
either way. If you install with a different `PREFIX`, pass the same `PREFIX` to
`make install-agent`.

## How it works

Parses the embedded cape plist, decodes each cursor's images, and registers them with
WindowServer through the private CoreGraphics cursor API. The Arrow and I-beam
cursors are also registered under every system cursor name containing
"arrow"/"ibeam", since macOS 26 renamed them.

## Credits

The cursor set (`macos_15_cursors.cape`) is the "MacOS 15" cape by Gino Xiscatti.

The approach and the private API usage come from
[Mousecape](https://github.com/alexzielenski/Mousecape) by Alex Zielenski and
the Tahoe-support fork this was derived from. No Mousecape source is included.
