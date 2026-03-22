#include "ui/preview_pane.hpp"
#include "ui/icons.hpp"
#include "core/dir_entry.hpp"

#include <ftxui/dom/elements.hpp>
#include <filesystem>

using namespace ftxui;
namespace fs = std::filesystem;

static const char* kind_label(EntryKind k) {
    switch (k) {
        case EntryKind::Directory: return "directory";
        case EntryKind::Executable: return "executable";
        case EntryKind::Symlink: return "symlink";
        case EntryKind::Hidden: return "hidden";
        case EntryKind::File: return "file";
        default: return "unknown";
    }
}

ftxui::Element render_preview(const DirEntry* sel, const Theme& theme) {
    if (!sel) {
        return vbox({ text("") }) | flex;
    }

    const std::string big_icon = icon_for(*sel);

    std::string symlink_target;
    if (sel->kind == EntryKind::Symlink) {
        std::error_code ec;
        auto target = fs::read_symlink(sel->path, ec);
        if (!ec) symlink_target = target.string();
    }

    Elements rows;
    rows.push_back(text("") );
    rows.push_back(
        hbox({ text("  " + big_icon), text(sel->name) | bold | color(theme.file) })
    );
    rows.push_back(text("") );
    rows.push_back(separatorLight() | color(theme.border));
    rows.push_back(text("") );

    auto row = [&](const std::string& label, const std::string& val) {
        return hbox({
            text("  " + label + "  ") | color(theme.hidden) | size(WIDTH, EQUAL, 14),
            text(val) | color(theme.file),
        });
    };

    rows.push_back(row("type", kind_label(sel->kind)));
    rows.push_back(row("perms", sel->perms));

    if (sel->kind != EntryKind::Directory) {
        rows.push_back(row("size", fmt_size(sel->size)));
    }

    if (!symlink_target.empty()) {
        rows.push_back(row("→ target", symlink_target));
    }

    rows.push_back(text(""));
    rows.push_back(separatorLight() | color(theme.border));

    return vbox(rows) | flex;
}
