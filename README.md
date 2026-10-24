# cl2

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

"Core lib 2": a personal collection of Qt/C++ helper classes (logging,
persistence, ASTM E1394 parsing, image views, TCP/serial/HTTP helpers),
built as one shared library.

Status: older project (2015-2019), not maintained. Build files are set up for
Windows/Visual Studio; a build on current Qt versions has not been tested.

## Contents

Modules compiled by `cl2.pro`:

| Path      | Contents |
|-----------|----------|
| `log/`    | `clog` logging |
| `os/`     | `fstools`, `persistence`, `filewatcher` |
| `kvs/`    | `KVStore` key/value store |
| `xml/`    | XPath helper |
| `db/`     | `DBManager`, `ImgDB` (Qt SQL) |
| `vision/` | `imgproc` |
| `lis/`    | ASTM E1394 record classes and parser (`astmparser`, `ASTMRecord`, with JSON output), `LisLink`, `LisProxy`, `DataMapper` |
| `sec/`    | `AuthManager`; `CryptManager`, which calls an external `CSDES.exe` for en-/decryption |
| `gui/`    | image views (`ImageView`, `ImageGroupView`, `ExtImageViewer`, `ExtGView`), `MarkerItem`, `ScreenManager`, `ExtComboBox` |
| `com/`    | serial port (Windows/Linux), TCP server, HTTPS request/upload, web API connector |
| `app/`    | language/translation manager, `QmlClient`, `SIServer` (small TCP server emitting received requests) |

Not part of the library build: `gui/objectselector.cpp` and
`gui/valuecontrols.cpp` (commented out), `meth/fsm11/` (small C++11 state
machine with its own Qt test app), `test/` (QTestLib tests for `log` and `xml`).

## Build

`cl2.pro` uses `TEMPLATE = vclib`, i.e. qmake generates a Visual Studio
library project. `update_vsprj.bat` regenerates the projects for cl2,
`3rdparty/QSMTP` and `3rdparty/QtWebApp` with Qt 4.8.4. A 2015 commit says
"Ported to Qt5"; for Qt 5 the `QT += widgets` line in `cl2.pro` would have to
be enabled.

The library is built to `../build`; `output.pri` sets
`DESTDIR = C:/Development/build`.

## Third-party code

- `3rdparty/QSMTP` – SmtpClient for Qt by Tőkés Attila, LGPL 2.1
- `3rdparty/QtWebApp` – QtWebApp, LGPL (`3rdparty/QtWebApp/doc/copyright.txt`, `lgpl-3.0.txt`)

## License

[MIT](LICENSE), (c) 2015 Valentin Heinitz, except the third-party code above.
