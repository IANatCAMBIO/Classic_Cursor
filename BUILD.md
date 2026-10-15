# Building classic_cursor

- Toolchain: Apple clang from Xcode Command Line Tools (any recent version; C99).
- `macos_15_cursors.cape` is embedded into the binary by the linker (`-sectcreate __DATA cape`);
  editing it requires a rebuild.
- Dependencies: system frameworks only — ApplicationServices, ImageIO, CoreFoundation.
- Targets: `make` (build + strip + sign), `make run`, `make install` (`PREFIX`, default
  `/usr/local`), `make install-agent` / `make uninstall-agent` (per-user LaunchAgent,
  run after `make install`), `make clean`. No tests.
- Signing: `make` signs with the hardened runtime using the keychain identity
  `SIGN_ID` (default `Apple Development`, which matches a single Apple Development
  certificate; pass the full name if you have several). An ad-hoc-signed build is
  killed and quarantined on exec by CrowdStrike Falcon; the Apple Development
  signature is not. `SIGN_ID=-` builds ad-hoc. Build as your user before
  `sudo make install` — root cannot reach your login keychain.
- Platform: macOS only. Uses undocumented CoreGraphics symbols
  (`CGSRegisterCursorWithImages`, `CGSCursorNameForSystemCursor`,
  `CGSSetDockCursorOverride`, `CGSMainConnectionID`) declared by hand in
  `classic_cursor.c`; they may change between macOS releases. Developed against macOS 26.
