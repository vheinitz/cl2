# cl2 — Core Library for Qt/C++ 🔧

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C%2B%2B-blue.svg)](#)

**Core lib 2** — a collection of reusable classes that extend Qt for the
kinds of tasks that keep coming up in real desktop and lab-device software.
Extracted from production machine-vision and LIS projects so the same
non-trivial code isn't rewritten in every new app.

## Modules

| Area | Path | What it offers |
|------|------|----------------|
| **LIS / ASTM** | `lis/` | `ASTM_Record`, `astmparser`, `LisLink`, `LisProxy`, `DataMapper` — ASTM E1394 parsing & LIS communication in Qt |
| **Image DB** | `db/` | `ImgDB`, `DBManager` — storing and querying image data |
| **GUI** | `gui/` | `ImageView`, `ImageGroupView`, `ScreenManager`, `ObjectSelector`, `markeritem`, `valuecontrols` — image-centric widgets for vision apps |
| **Security** | `sec/` | `CryptManager`, `AuthManager` — AES/encryption + authentication |
| **XML** | `xml/` | `xpath` helpers |
| **Persistence** | `os/` | `persistence`, `fstools` — app-state and filesystem helpers |
| **Key-value store** | `kvs/` | `KVStore` — simple key/value storage |
| **Logging** | `log/` | `clog` — console/log helpers |

## Highlights

- **`lis/astmparser`** — the same ASTM E1394 parsing expertise found in
  [GoodnightAstm](../GoodnightAstm) and [ASTMViewer](../ASTMViewer), reworked
  as a clean Qt/C++ `QSharedPointer`-based object tree.
- **`gui/`** — building blocks for image-selection UIs (image lists, viewers,
  object selectors) that appear in classification and annotation tools such as
  [Classifier2](../Classifier2).

## Requirements

- Qt 5 (Core, Gui, Widgets, Sql, Xml)
- C++11+ compiler

## Build

The project is a `qmake` `.pro` library:

```bash
qmake && make
```

Use `output.pri` to include it in your own `qmake` project.

## License

[MIT](LICENSE) © Valentin Heinitz
