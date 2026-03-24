#include "ui/file_pane.hpp"
#include "ui/icons.hpp"
#include "core/bookmarks.hpp"
#include "core/fs_ops.hpp"
#include "input/keybinds.hpp"
#include "core/dir_entry.hpp"
#include "ui/theme.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

using namespace ftxui;
namespace fs = std::filesystem;

static std::string display_name(const DirEntry& e) {
    if (e.kind == EntryKind::Directory) return e.name + "/";
    if (e.kind == EntryKind::Symlink) return e.name + " →";
    return e.name;
}

enum class ModalKind { None, ConfirmDelete, Rename, NewFile, NewDir };

enum class PendingMode { None, BookmarkAdd, BookmarkJump };

struct ModalData {
    ModalKind kind = ModalKind::None;
    std::string rename_buf;
    std::string new_entry_buf;
};

struct FilterData {
    bool active = false;
    std::string buf;
    std::vector<DirEntry> base;
};

static Element render_delete_dialog(const std::string& name, const Theme& t) {
    return vbox({
        text(""),
        text("  Delete '" + name + "' ?") | bold | color(Color::White),
        text(""),
        hbox({
            text("   [y] ") | color(Color::RGB(255, 80, 80)) | bold,
            text("confirm      "),
            text("[n]  ") | color(t.hidden),
            text("cancel"),
        }),
        text(""),
    }) | border | color(t.border) | bgcolor(Color::RGB(18, 18, 18));
}

static Element render_rename_dialog(const std::string& buf, const Theme& t) {
    return vbox({
        text(""),
        text("  Rename") | bold | color(Color::White),
        text(""),
        hbox({ text("  > ") | color(t.dir), text(buf + "_") | color(Color::White) }),
        text(""),
        hbox({
            text("   [Enter] ") | color(t.exec) | bold,
            text("confirm      "),
            text("[Esc]  ") | color(t.hidden),
            text("cancel"),
        }),
        text(""),
    }) | border | color(t.border) | bgcolor(Color::RGB(18, 18, 18));
}

static Element render_new_entry_dialog(const std::string& buf, bool is_dir, const Theme& t) {
    return vbox({
        text(""),
        text(is_dir ? "  New directory" : "  New file") | bold | color(Color::White),
        text(""),
        hbox({ text("  > ") | color(t.dir), text(buf + "_") | color(Color::White) }),
        text(""),
        hbox({
            text("   [Enter] ") | color(t.exec) | bold,
            text("confirm      "),
            text("[Esc]  ") | color(t.hidden),
            text("cancel"),
        }),
        text(""),
    }) | border | color(t.border) | bgcolor(Color::RGB(18, 18, 18));
}

ftxui::Component make_file_pane(FilePaneState& state, const Theme& theme) {
    auto md = std::make_shared<ModalData>();
    auto fd = std::make_shared<FilterData>();
    auto pending = std::make_shared<PendingMode>(PendingMode::None);

    auto component = Renderer([&, md, fd, pending] {
        auto header = hbox({
            text(" Name") | color(theme.pane_title) | flex,
            text("Perms     ") | color(theme.pane_title) | size(WIDTH, EQUAL, 11),
            text("Size     ") | color(theme.pane_title) | size(WIDTH, EQUAL, 9),
        });

        Elements rows;
        rows.push_back(header);
        rows.push_back(separatorLight() | color(theme.border));

        for (int i = 0; i < (int)state.entries.size(); ++i) {
            const auto& e = state.entries[i];
            const bool sel = (i == state.cursor);
            const std::string label = icon_for(e) + display_name(e);

            auto name_col = style_entry(theme, e.kind, sel, text(" " + label)) | flex;
            auto perms_col = style_entry(theme, e.kind, sel, text(e.perms)) | size(WIDTH, EQUAL, 11);
            auto size_col = style_entry(theme, e.kind, sel, text(e.kind == EntryKind::Directory ? "         " : fmt_size(e.size))) | size(WIDTH, EQUAL, 9);

            auto row = hbox({ name_col, perms_col, size_col });
            if (sel) row = row | focus;
            rows.push_back(row);
        }

        if (state.entries.empty())
            rows.push_back(text("  (empty)") | dim);

        if (state.clipboard) {
            const auto clip_name = fs::path(state.clipboard->path).filename().string();
            rows.push_back(separatorLight() | color(theme.border));
            rows.push_back(hbox({
                text(state.clipboard->is_cut ? "  cut:  " : "  yank: ") | color(theme.hidden),
                text(clip_name) | color(theme.symlink),
            }));
        }

        Element list_el = vbox(rows) | frame | flex;

        Element base_el = list_el;
        if (md->kind == ModalKind::ConfirmDelete) {
            const std::string name = !state.entries.empty() ? state.entries[state.cursor].name : "";
            base_el = dbox({ list_el, render_delete_dialog(name, theme) | clear_under | center });
        } else if (md->kind == ModalKind::Rename) {
            base_el = dbox({ list_el, render_rename_dialog(md->rename_buf, theme) | clear_under | center });
        } else if (md->kind == ModalKind::NewFile || md->kind == ModalKind::NewDir) {
            base_el = dbox({ list_el, render_new_entry_dialog(md->new_entry_buf, md->kind == ModalKind::NewDir, theme) | clear_under | center });
        }

        if (*pending != PendingMode::None) {
            const bool is_add = (*pending == PendingMode::BookmarkAdd);
            auto prompt = text(is_add ? " m  " : " ;  ") | color(theme.dir) | bold;
            auto label = text(is_add ? "mark as: _" : "jump to: _") | color(theme.file);
            auto bar = hbox({ prompt, label | flex });
            return vbox({
                base_el,
                separator() | color(theme.border),
                bar,
            });
        }

        if (!fd->active) return base_el;

        auto prompt = text(" /  ") | color(theme.dir) | bold;
        auto query = text(fd->buf + "_") | color(theme.file);
        auto bar = hbox({ prompt, query | flex });

        return vbox({
            base_el,
            separator() | color(theme.border),
            bar,
        });
    });

    return CatchEvent(component, [&, md, fd, pending](Event event) -> bool {
        const int n = (int)state.entries.size();

        if (fd->active) {
            if (event == Event::Escape) {
                state.entries = fd->base;
                state.cursor = 0;
                fd->active = false;
                fd->buf.clear();
                return true;
            }
            if (event == Event::Return) {
                fd->active = false;
                return true;
            }
            if (event == Event::Backspace) {
                if (!fd->buf.empty()) fd->buf.pop_back();
                if (fd->buf.empty()) {
                    state.entries = fd->base;
                } else {
                    std::string lower_buf = fd->buf;
                    std::transform(lower_buf.begin(), lower_buf.end(), lower_buf.begin(), ::tolower);
                    state.entries.clear();
                    for (const auto& e : fd->base) {
                        std::string lower_name = e.name;
                        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
                        if (lower_name.find(lower_buf) != std::string::npos)
                            state.entries.push_back(e);
                    }
                }
                state.cursor = 0;
                return true;
            }
            if (event.is_character()) {
                fd->buf += event.character();
                std::string lower_buf = fd->buf;
                std::transform(lower_buf.begin(), lower_buf.end(), lower_buf.begin(), ::tolower);
                state.entries.clear();
                for (const auto& e : fd->base) {
                    std::string lower_name = e.name;
                    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
                    if (lower_name.find(lower_buf) != std::string::npos)
                        state.entries.push_back(e);
                }
                state.cursor = 0;
                return true;
            }
            return true;
        }

        if (*pending != PendingMode::None) {
            if (event == Event::Escape) {
                *pending = PendingMode::None;
                return true;
            }
            if (event.is_character()) {
                const char key = event.character()[0];
                if (*pending == PendingMode::BookmarkAdd) {
                    if (state.bookmarks) {
                        (*state.bookmarks)[key] = state.cwd;
                        save_bookmarks(*state.bookmarks);
                        state.status_msg = std::string("Bookmarked '") + key + "': " + state.cwd;
                    }
                } else {
                    if (state.bookmarks) {
                        auto it = state.bookmarks->find(key);
                        if (it != state.bookmarks->end()) {
                            state.cwd = it->second;
                            state.entries = scan_dir(state.cwd);
                            state.cursor = 0;
                            state.status_msg.clear();
                        } else {
                            state.status_msg = std::string("No bookmark '") + key + "'";
                        }
                    }
                }
                *pending = PendingMode::None;
                return true;
            }
            return true;
        }

        if (md->kind == ModalKind::ConfirmDelete) {
            if (event == Event::Character('y')) {
                std::string err;
                const auto& sel = state.entries[state.cursor];
                if (fs_ops::delete_path(sel.path, err)) {
                    state.status_msg = "Deleted: " + sel.name;
                } else {
                    state.status_msg = "Error: " + err;
                }
                state.entries = scan_dir(state.cwd);
                state.cursor = std::min(state.cursor, (int)state.entries.size() - 1);
                if (state.cursor < 0) state.cursor = 0;
                md->kind = ModalKind::None;
            } else if (event == Event::Character('n') || event == Event::Escape) {
                md->kind = ModalKind::None;
            }
            return true;
        }

        if (md->kind == ModalKind::Rename) {
            if (event == Event::Return) {
                if (!md->rename_buf.empty()) {
                    std::string err;
                    const auto& sel = state.entries[state.cursor];
                    if (fs_ops::rename_entry(sel.path, md->rename_buf, err)) {
                        state.status_msg = "Renamed to: " + md->rename_buf;
                        state.entries = scan_dir(state.cwd);
                        for (int i = 0; i < (int)state.entries.size(); ++i) {
                            if (state.entries[i].name == md->rename_buf) {
                                state.cursor = i; break;
                            }
                        }
                    } else {
                        state.status_msg = "Error: " + err;
                    }
                }
                md->kind = ModalKind::None;
                return true;
            }
            if (event == Event::Escape) { md->kind = ModalKind::None; return true; }
            if (event == Event::Backspace) {
                if (!md->rename_buf.empty()) md->rename_buf.pop_back();
                return true;
            }
            if (event.is_character()) { md->rename_buf += event.character(); return true; }
            return true;
        }

        if (md->kind == ModalKind::NewFile || md->kind == ModalKind::NewDir) {
            if (event == Event::Return) {
                if (!md->new_entry_buf.empty()) {
                    std::string err;
                    const fs::path target = fs::path(state.cwd) / md->new_entry_buf;
                    bool ok = true;
                    if (md->kind == ModalKind::NewDir) {
                        std::error_code ec;
                        fs::create_directory(target, ec);
                        if (ec) { err = ec.message(); ok = false; }
                    } else {
                        std::ofstream f(target);
                        if (!f) { err = "could not create file"; ok = false; }
                    }
                    state.entries = scan_dir(state.cwd);
                    state.status_msg = ok ? "Created: " + md->new_entry_buf : "Error: " + err;
                    if (ok) {
                        for (int i = 0; i < (int)state.entries.size(); ++i) {
                            if (state.entries[i].name == md->new_entry_buf) {
                                state.cursor = i; break;
                            }
                        }
                    }
                }
                md->kind = ModalKind::None;
                return true;
            }
            if (event == Event::Escape) { md->kind = ModalKind::None; return true; }
            if (event == Event::Backspace) {
                if (!md->new_entry_buf.empty()) md->new_entry_buf.pop_back();
                return true;
            }
            if (event.is_character()) { md->new_entry_buf += event.character(); return true; }
            return true;
        }

        const Action act = resolve_action(event);

        switch (act) {
            case Action::MoveUp:
                state.cursor = std::max(0, state.cursor - 1);
                state.status_msg.clear();
                return true;

            case Action::MoveDown:
                state.cursor = std::min(n - 1, state.cursor + 1);
                state.status_msg.clear();
                return true;

            case Action::PageUp:
                state.cursor = std::max(0, state.cursor - 15);
                return true;

            case Action::PageDown:
                state.cursor = std::min(n - 1, state.cursor + 15);
                return true;

            case Action::Enter:
            case Action::Back: {
                std::string target;
                if (act == Action::Back) {
                    if (state.cwd == "/") return true;
                    target = fs::path(state.cwd).parent_path().string();
                } else {
                    if (state.entries.empty()) return true;
                    const auto& sel = state.entries[state.cursor];
                    if (sel.kind == EntryKind::Directory) {
                        target = sel.path;
                    } else {
                        if (state.open_callback) state.open_callback(sel.path);
                        return true;
                    }
                }
                state.cwd = target;
                state.entries = scan_dir(target);
                state.cursor = 0;
                state.status_msg.clear();
                return true;
            }

            case Action::Open: {
                if (state.entries.empty()) return true;
                const auto& sel = state.entries[state.cursor];
                if (sel.kind != EntryKind::Directory && state.open_callback)
                    state.open_callback(sel.path);
                return true;
            }

            case Action::Delete: {
                if (n == 0) return true;
                if (state.entries[state.cursor].name == "..") return true;
                md->kind = ModalKind::ConfirmDelete;
                return true;
            }

            case Action::Rename: {
                if (n == 0) return true;
                if (state.entries[state.cursor].name == "..") return true;
                md->rename_buf = state.entries[state.cursor].name;
                md->kind = ModalKind::Rename;
                return true;
            }

            case Action::Yank: {
                if (n == 0 || state.entries[state.cursor].name == "..") return true;
                state.clipboard = ClipboardEntry{ state.entries[state.cursor].path, false };
                state.status_msg = "Yanked: " + state.entries[state.cursor].name;
                return true;
            }

            case Action::Cut: {
                if (n == 0 || state.entries[state.cursor].name == "..") return true;
                state.clipboard = ClipboardEntry{ state.entries[state.cursor].path, true };
                state.status_msg = "Cut: " + state.entries[state.cursor].name;
                return true;
            }

            case Action::Paste: {
                if (!state.clipboard) {
                    state.status_msg = "Nothing to paste";
                    return true;
                }
                std::string err;
                bool ok;
                if (state.clipboard->is_cut) {
                    ok = fs_ops::move_path(state.clipboard->path, state.cwd, err);
                    if (ok) state.clipboard.reset();
                } else {
                    ok = fs_ops::copy_path(state.clipboard->path, state.cwd, err);
                }
                state.entries = scan_dir(state.cwd);
                state.status_msg = ok ? "Pasted successfully" : "Error: " + err;
                return true;
            }

            case Action::NewFile: {
                md->new_entry_buf.clear();
                md->kind = ModalKind::NewFile;
                return true;
            }

            case Action::NewDir: {
                md->new_entry_buf.clear();
                md->kind = ModalKind::NewDir;
                return true;
            }

            case Action::Search: {
                if (md->kind != ModalKind::None) return true;
                fd->active = true;
                fd->buf.clear();
                fd->base = state.entries;
                state.cursor = 0;
                return true;
            }

            case Action::BookmarkAdd: {
                if (md->kind != ModalKind::None || fd->active) return true;
                *pending = PendingMode::BookmarkAdd;
                return true;
            }

            case Action::BookmarkJump: {
                if (md->kind != ModalKind::None || fd->active) return true;
                *pending = PendingMode::BookmarkJump;
                return true;
            }

            default:
                return false;
        }
    });
}
