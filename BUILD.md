# Building RetroCursor

- Toolchain: Apple clang from Xcode Command Line Tools (any recent version; C99).
- `cursors.cape` is embedded into the binary by the linker (`-sectcreate __DATA cape`);
  editing it requires a rebuild.
- Dependencies: system frameworks only — ApplicationServices, ImageIO, CoreFoundation.
- Targets: `make` (build + strip), `make run`, `make install` (`PREFIX`, default
  `/usr/local`), `make install-agent` / `make uninstall-agent` (per-user LaunchAgent,
  run after `make install`), `make clean`. No tests.
- Platform: macOS only. Uses undocumented CoreGraphics symbols
  (`CGSRegisterCursorWithImages`, `CGSCursorNameForSystemCursor`,
  `CGSSetDockCursorOverride`, `CGSMainConnectionID`) declared by hand in
  `retrocursor.c`; they may change between macOS releases. Developed against macOS 26.
