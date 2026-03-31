# tfm
A keyboard-driven TUI file manager for Linux, built with C++17 and FTXUI.

## Screenshot on my setup
### Default
<img width="1920" height="1200" alt="Screenshot_20260325_000455" src="https://github.com/user-attachments/assets/160fe70f-89e1-4a6f-8f00-475872231a14" />

### Me playing with TOML config

#### KDE Plasma
<img width="2559" height="1550" alt="Screenshot_20260326_171450" src="https://github.com/user-attachments/assets/eba45c0f-e346-428d-b72a-612f70165738" />

#### Hyprland
<img width="2560" height="1558" alt="20260329_151722" src="https://github.com/user-attachments/assets/2ab5304d-c019-4fc0-a7b9-2fcece36b28f" />

> **Note:** A Nerd Font is required for icons to render correctly (e.g. `ttf-jetbrains-mono-nerd`).

## Features
- **Vim-like bindings:** Navigate your filesystem without leaving the home row. (mostly)
- **Nerd Font icons:** File type icons for entries and the preview pane.
- **Rich preview pane:** Text files, directory listings, audio metadata, image dimensions, and ELF/binary hex dumps.
- **Fuzzy filter:** Filter the current directory in real time.
- **50/50 split layout:** Equal file list and preview pane.
- **Bookmarks:** Save directories with `m`+key and jump back with `;`+key, persisted across sessions.
- **Shell integration:** Quit with `q` and your terminal cds to wherever you were browsing.
- **TOML config:** Rebind any key and customize every color via `~/.config/tfm/config.toml`.
- **Git indicators:** Minimal per-file git status badges (M, A, D, ?) when inside a repo. Requires git on PATH.

## Dependencies
- `ftxui` — pulled automatically via CMake FetchContent, no manual install needed
- `toml++` — pulled automatically via CMake FetchContent, no manual install needed
- `taglib` — for audio file metadata (title, artist, album, duration, bitrate)
- `git` — for git status indicators (implicitly optional)

## Installation
### Arch Linux

```bash
yay -S tfm-git
```

### Other Distros

Scroll lower to see how to build from source.

## Build

### Arch Linux (tested)
```bash
sudo pacman -S ninja cmake gcc taglib
cmake --preset release
cmake --build build
sudo cmake --install build
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
| <kbd>o</kbd> | Open editor picker |
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

### Bookmarks
| Key | Action |
|-----|--------|
| <kbd>m</kbd> + key | Bookmark current directory under key (a–z, 0–9) |
| <kbd>;</kbd> + key | Jump to bookmarked directory |

> Bookmarks are persisted to `~/.config/tfm/bookmarks` (respects `$XDG_CONFIG_HOME`).

## Configuration

tfm reads an optional TOML config file on startup in `~/.config/tfm/config.toml`

> If you have `$XDG_CONFIG_HOME` set, tfm respects it \
> The directory is not created automatically, create it yourself if it doesn't exist


### Keybindings

Keybindings are configured under the `[keys]` table. Example:

```toml
[keys]
MoveUp   = ["k", "ArrowUp"]
MoveDown = ["j", "ArrowDown"]
Delete   = ["D"]
Quit     = ["q", "Q"]
```

> **Note:** When you specify an action in the config, your array fully replaces the defaults for that action. For special keys use: "ArrowUp", "ArrowDown", "ArrowLeft", "ArrowRight", "Enter", "Backspace", "PageUp", "PageDown", "Escape"

#### Available action names

`MoveUp`, `MoveDown`, `PageUp`, `PageDown`, `Enter`, `Back`, `Open`, `Delete`, `Rename`, `Yank`, `Cut`, `Paste`, `NewFile`, `NewDir`, `Search`, `BookmarkAdd`, `BookmarkJump`, `Quit`

### Theme

Colors are configured under the `[theme]` table. Every value must be a hex color string. Example:

```toml
[theme]
dir         = "#5fd7ff"
selected_bg = "#005faf"
border      = "#444444"
```

#### Available color slots

`dir`, `hidden_dir`, `file`, `exec`, `symlink`, `hidden`, `selected_bg`, `selected_fg`, `status_fg`, `status_bg`, `border`, `pane_title`

## Shell integration

After installing just type `tfm` to launch it from anywhere.

To also have it cd your shell to the last directory on quit, add a small wrapper function for your shell.

**fish** — save as `~/.config/fish/functions/tfm.fish`:
```fish
function tfm
    set tmp (mktemp)
    command tfm --cd-file $tmp $argv
    set dest (string trim (cat $tmp 2>/dev/null))
    rm -f $tmp
    if test -n "$dest" && test "$dest" != "$PWD"
        cd $dest
    end
end
```

**zsh** — add to `~/.zshrc`:
```zsh
tfm() {
    local tmp
    tmp=$(mktemp)
    command tfm --cd-file "$tmp" "$@"
    local dest
    dest=$(cat "$tmp" 2>/dev/null)
    rm -f "$tmp"
    [ -n "$dest" ] && [ "$dest" != "$PWD" ] && cd "$dest"
}
```

**bash** — add to `~/.bashrc`:
```bash
tfm() {
    local tmp
    tmp=$(mktemp)
    command tfm --cd-file "$tmp" "$@"
    local dest
    dest=$(cat "$tmp" 2>/dev/null)
    rm -f "$tmp"
    [ -n "$dest" ] && [ "$dest" != "$PWD" ] && cd "$dest"
}
```

Once set up, just run `tfm` and browse around!

## Git indicators

Intentionally minimal, it just shells out to `git status --porcelain` on each directory load. That said, this is an open-source project and if anyone feels like expanding the git integration further, contributions are very welcome.


## Project structure
```
tfm/
├── CMakeLists.txt          — build config
├── CMakePresets.json       — cmake --preset release/debug shortcuts
├── include/
│   ├── app/app.hpp         — declares run_app(cd_file)
│   ├── core/
│   │   ├── bookmarks.hpp   — BookmarkMap, load_bookmarks(), save_bookmarks()
│   │   ├── config.hpp      — Config struct, load_config()
│   │   ├── dir_entry.hpp   — DirEntry, EntryKind, scan_dir(), fmt_size()
│   │   └── fs_ops.hpp      — copy, move, rename, delete, open_in_editor()
│   ├── input/keybinds.hpp  — Action enum, event_to_key(), resolve_action()
│   └── ui/
│       ├── file_pane.hpp   — FilePaneState, ClipboardEntry, make_file_pane()
│       ├── icons.hpp       — icon_for()
│       ├── preview_pane.hpp— render_preview()
│       └── theme.hpp       — Theme struct, default_theme(), style_entry()
└── src/
    ├── main.cpp            — parses --cd-file / $TFM_CD_FILE, calls run_app()
    ├── app/app.cpp         — screen loop, loads config+bookmarks, wires all modules, writes cd-file on quit
    ├── core/
    │   ├── bookmarks.cpp   — XDG-aware bookmark persistence (~/.config/tfm/bookmarks)
    │   ├── config.cpp      — TOML config parsing, default key map and theme
    │   ├── dir_entry.cpp   — directory scanning via std::filesystem + POSIX lstat
    │   └── fs_ops.cpp      — filesystem operations
    ├── input/keybinds.cpp  — FTXUI events → key strings → Actions
    └── ui/
        ├── file_pane.cpp   — file list, navigation, filter, dialogs, clipboard, bookmarks
        ├── icons.cpp       — Nerd Font icon lookup by kind and extension
        ├── preview_pane.cpp— text/image/audio/binary/directory preview
        └── theme.cpp       — default colors
```

## Roadmap
- [x] Step 1 — Foundation: directory listing, navigation, theme
- [x] Step 2 — Two-pane layout, Nerd Font icons, preview pane skeleton
- [x] Step 3 — File operations: open, create, rename, delete, yank/cut/paste
- [x] Step 4 — Preview pane content: text preview, directory listings, audio metadata, image dimensions, binary/ELF hex dump
- [x] Step 5 — Real-time fuzzy filter
- [x] Step 6 — Shell integration, bookmarks
- [x] Step 7 — TOML config, rebindable keys, custom themes
- [x] Step 8 — Polish, git indicators
- [x] Step 9 — AUR package

## Contributing

tfm started as a personal tool and the original roadmap is done, but there's always more to build. I myself will probably come back to add more if I miss something. If you find a bug, have a feature idea, or want to add support for your distro, issues and contributions are very welcome.

Some ideas if you're looking for a starting point:
- Support for more distros (currently only tested on Arch since that's what I use)
- Bulk operations (multi-select)
- More preview types
- Expanded git integration (it's intentionally minimal right now)

I hope tfm serves you well, and thank you for reading to the end.
