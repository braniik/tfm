# tfm
A keyboard-driven TUI file manager for Linux, built with C++17 and FTXUI.

## Features
- **Vim-like bindings:** Navigate your filesystem without leaving the home row. (mostly)
- **Nerd Font icons:** File type icons for entries and the preview pane.
- **Rich preview pane:** Text files, directory listings, audio metadata, image dimensions, and ELF/binary hex dumps.
- **Fuzzy filter:** Filter the current directory in real time.
- **50/50 split layout:** Equal file list and preview pane.

## Dependencies
- `ftxui` — pulled automatically via CMake FetchContent, no manual install needed
- `taglib` — for audio file metadata (title, artist, album, duration, bitrate)

## Build

### Arch Linux (tested)
```bash
sudo pacman -S ninja cmake gcc taglib
cmake --preset release
cmake --build build
./build/tfm
```

### Debug build
```bash
cmake --preset debug
cmake --build build-debug
./build-debug/tfm
```

### Other distros (vcpkg fallback)
```bash
git clone https://github.com/microsoft/vcpkg ~/.vcpkg
~/.vcpkg/bootstrap-vcpkg.sh
cmake --preset release -DCMAKE_TOOLCHAIN_FILE=$HOME/.vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

## Keys

### Navigation
| Key | Action |
|-----|--------|
| <kbd>j</kbd> / <kbd>↓</kbd> | Move down |
| <kbd>k</kbd> / <kbd>↑</kbd> | Move up |
| <kbd>l</kbd> / <kbd>→</kbd> / <kbd>Enter</kbd> | Enter directory |
| <kbd>h</kbd> / <kbd>←</kbd> / <kbd>Backspace</kbd> | Go up a directory |
| <kbd>PgDn</kbd> | Jump down 15 entries |
| <kbd>PgUp</kbd> | Jump up 15 entries |
| <kbd>q</kbd> | Quit |

### File operations
| Key | Action |
|-----|--------|
| <kbd>o</kbd> | Open file in `$EDITOR` |
| <kbd>n</kbd> | New file |
| <kbd>N</kbd> | New directory |
| <kbd>r</kbd> | Rename |
| <kbd>d</kbd> | Delete |
| <kbd>y</kbd> | Yank |
| <kbd>x</kbd> | Cut |
| <kbd>p</kbd> | Paste |

### Search
| Key | Action |
|-----|--------|
| <kbd>/</kbd> | Open filter bar |
| <kbd>Esc</kbd> | Close filter, restore full listing |
| <kbd>Enter</kbd> | Close filter bar, keep results |

## Project structure
```
tfm/
├── CMakeLists.txt          — build config
├── CMakePresets.json       — cmake --preset release/debug shortcuts
├── include/
│   ├── app/app.hpp         — declares run_app()
│   ├── core/
│   │   ├── dir_entry.hpp   — DirEntry, EntryKind, scan_dir(), fmt_size()
│   │   └── fs_ops.hpp      — copy, move, rename, delete, open_in_editor()
│   ├── input/keybinds.hpp  — Action enum, resolve_action()
│   └── ui/
│       ├── file_pane.hpp   — FilePaneState, ClipboardEntry, make_file_pane()
│       ├── icons.hpp       — icon_for()
│       ├── preview_pane.hpp— render_preview()
│       └── theme.hpp       — Theme struct, default_theme(), style_entry()
└── src/
    ├── main.cpp            — calls run_app()
    ├── app/app.cpp         — screen loop, wires all modules, owns the layout
    ├── core/
    │   ├── dir_entry.cpp   — directory scanning via std::filesystem + POSIX lstat
    │   └── fs_ops.cpp      — filesystem operations
    ├── input/keybinds.cpp  — raw FTXUI events → named Actions
    └── ui/
        ├── file_pane.cpp   — file list, navigation, filter, dialogs, clipboard
        ├── icons.cpp       — Nerd Font icon lookup by kind and extension
        ├── preview_pane.cpp— text/image/audio/binary/directory preview
        └── theme.cpp       — every color in the app lives here
```

## Requirements
- Linux
- GCC with C++17 support
- CMake 3.20+
- Ninja
- TagLib
- A [Nerd Font](https://www.nerdfonts.com/) (e.g. `ttf-jetbrains-mono-nerd` from pacman)

## Roadmap
- [x] Step 1 — Foundation: directory listing, navigation, theme
- [x] Step 2 — Two-pane layout, Nerd Font icons, preview pane skeleton
- [x] Step 3 — File operations: open, create, rename, delete, yank/cut/paste
- [x] Step 4 — Preview pane content: text preview, directory listings, audio metadata, image dimensions, binary/ELF hex dump
- [x] Step 5 — Real-time fuzzy filter
- [ ] Step 6 — Shell integration, bookmarks
- [ ] Step 7 — TOML config, rebindable keys, custom themes
- [ ] Step 8 — Polish, git indicators
- [ ] Step 9 — AUR package (maybe)
