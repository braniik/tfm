#pragma once

#include <ftxui/component/component.hpp>
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include "core/bookmarks.hpp"
#include "core/dir_entry.hpp"
#include "input/keybinds.hpp"
#include "ui/theme.hpp"

struct ClipboardEntry {
    std::string path;
    bool is_cut = false;
};

struct FilePaneState {
    std::string cwd;
    std::vector<DirEntry> entries;
    int cursor = 0;

    std::optional<ClipboardEntry> clipboard;
    std::string status_msg;
    std::function<void(const std::string&)> open_callback;

    BookmarkMap *bookmarks = nullptr;
};

ftxui::Component make_file_pane(FilePaneState &state, const Theme &theme, const std::unordered_map<std::string, Action> &key_map);
