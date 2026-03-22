#pragma once

#include <ftxui/component/component.hpp>
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include "core/dir_entry.hpp"
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
};

ftxui::Component make_file_pane(FilePaneState& state, const Theme& theme);
