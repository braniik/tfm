# tfm
A keyboard-driven TUI file manager for Linux, built with C++17 and FTXUI.

## Features
* **Vim-like Bindings:** Navigate your filesystem without leaving the home row. (mostly)
* **Nerd Font Support:** Awesome icons for files and folders.
* **Integrated Preview:** Quickly view file metadata and symlink targets.
* **Zero-Dependency UI:** FTXUI is bundled via CMake so just build and run.

## Build

### Arch Linux (tested)
```bash
sudo pacman -S ninja cmake gcc
cmake --preset release
cmake --build build
./build/tfm
```
> ftxui is pulled automatically via CMake FetchContent, no system install needed.

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
        ├── file_pane.cpp   — file list, navigation, dialogs, clipboard
        ├── icons.cpp       — Nerd Font icon lookup by kind and extension
        ├── preview_pane.cpp— right pane, shows type/perms/size/symlink target
        └── theme.cpp       — every color in the app lives here
```

## Requirements
- Linux
- GCC with C++17 support
- CMake 3.20+
- Ninja
- A [Nerd Font](https://www.nerdfonts.com/) (e.g. `ttf-jetbrains-mono-nerd` from pacman)

## Roadmap
- [x] Step 1 — Foundation: directory listing, navigation, theme
- [x] Step 2 — Two-pane layout, Nerd Font icons, preview pane skeleton
- [x] Step 3 — File operations: open, create, rename, delete, yank/cut/paste
- [ ] Step 4 — Preview pane content: text preview, directory stats, kitty image protocol
- [ ] Step 5 — Fuzzy search and filtering
- [ ] Step 6 — Shell integration, bookmarks, custom commands
- [ ] Step 7 — TOML config, rebindable keys, custom themes
- [ ] Step 8 — Polish, git indicators
- [ ] Step 9 — AUR package (maybe)
