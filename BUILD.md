# Building RetroCursor

- Toolchain: Apple clang from Xcode Command Line Tools (any recent version; C99).
- Dependencies: system frameworks only — ApplicationServices, ImageIO, CoreFoundation.
- Targets: `make` (build + strip), `make run`, `make install` (`PREFIX`, default
  `/usr/local`), `make clean`. No tests.
- Platform: macOS only. Uses undocumented CoreGraphics symbols
  (`CGSRegisterCursorWithImages`, `CGSCursorNameForSystemCursor`,
  `CGSSetDockCursorOverride`, `CGSMainConnectionID`) declared by hand in
  `retrocursor.c`; they may change between macOS releases. Developed against macOS 26.
