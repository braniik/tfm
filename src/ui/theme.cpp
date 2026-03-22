#include "ui/theme.hpp"

using namespace ftxui;

static const Theme DARK = {
    /* dir         */ Color::RGB(80, 210, 255),
    /* file        */ Color::White,
    /* exec        */ Color::RGB(90, 255, 90),
    /* symlink     */ Color::RGB(255, 190, 60),
    /* hidden      */ Color::RGB(105, 105, 105),
    /* selected_bg */ Color::RGB(25, 85, 195),
    /* selected_fg */ Color::White,
    /* status_fg   */ Color::RGB(200, 200, 200),
    /* status_bg   */ Color::RGB(18, 18, 18),
    /* border      */ Color::RGB(55, 55, 55),
    /* pane_title  */ Color::RGB(150, 150, 150),
};

const Theme& default_theme() { return DARK; }

Element style_entry(const Theme& t, EntryKind kind, bool selected, Element elem) {
    if (selected) {
        return elem | color(t.selected_fg) | bgcolor(t.selected_bg) | bold;
    }
    switch (kind) {
        case EntryKind::Directory: return elem | color(t.dir) | bold;
        case EntryKind::Executable: return elem | color(t.exec) | bold;
        case EntryKind::Symlink: return elem | color(t.symlink);
        case EntryKind::Hidden: return elem | color(t.hidden) | dim;
        case EntryKind::File:
        default: return elem | color(t.file);
    }
}
