<p align="center">
  <img src="glove_cursor.png" alt="Classic glove cursor" width="150">
</p>

# classic_cursor

macOS 26 (Tahoe) and later replaced the system cursors with a new design.  
Classic_cursor is a tiny, simple patch that restores the classic cursors from macOS Big Sur and earlier.
Once the cursors are updated, the tool exits. 

Cursors stay replaced until logout or reboot. There is no backup/restore; run it
again at each login (see below).

## Install

Prerequisites: macOS with Xcode Command Line Tools (`xcode-select --install`) and an
Apple Development signing certificate in your login keychain (see BUILD.md, Signing).

```sh
make              # build and sign ./classic_cursor; run as your user
make run          # build and apply the classic cursors now
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


*`~/.local/bin` is not on PATH by default on macOS; add it to your shell profile if you
want to run `classic_cursor` by hand. The LaunchAgent uses the full path, so it works
either way. If you install with a different `PREFIX`, pass the same `PREFIX` to
`make install-agent`.

## How it works

Decodes the built-in cursor images and registers them with WindowServer through the
private CoreGraphics cursor API. The Arrow and I-beam cursors are also registered under
every system cursor name containing "arrow"/"ibeam", since macOS 26 renamed them.

## Credits

The cursor artwork is the "MacOS 15" set by Gino Xiscatti.

The method and private API usage come from
[Mousecape](https://github.com/alexzielenski/Mousecape) by Alex Zielenski
No Mousecape source is included.
