#pragma once

#include <ftxui/screen/color.hpp>
#include <ftxui/dom/elements.hpp>
#include "core/dir_entry.hpp"

struct Theme {
    ftxui::Color dir;
    ftxui::Color file;
    ftxui::Color exec;
    ftxui::Color symlink;
    ftxui::Color hidden;
    ftxui::Color selected_bg;
    ftxui::Color selected_fg;
    ftxui::Color status_fg;
    ftxui::Color status_bg;
    ftxui::Color border;
    ftxui::Color pane_title;
};

const Theme &default_theme();

ftxui::Element style_entry(const Theme &t, EntryKind kind, bool selected, ftxui::Element elem);
