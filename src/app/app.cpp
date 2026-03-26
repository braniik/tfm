#include "app/app.hpp"
#include "core/bookmarks.hpp"
#include "core/config.hpp"
#include "core/dir_entry.hpp"
#include "core/fs_ops.hpp"
#include "input/keybinds.hpp"
#include "ui/file_pane.hpp"
#include "ui/preview_pane.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

using namespace ftxui;
namespace fs = std::filesystem;

void run_app(std::string cd_file) {
    const Config cfg = load_config();
    const Theme &theme = cfg.theme;

    std::string cwd;
    if (const char *pwd = std::getenv("PWD")) {
        cwd = pwd;
    } else {
        cwd = fs::current_path().string();
    }

    BookmarkMap bookmarks = load_bookmarks();

    FilePaneState pane_state;
    pane_state.cwd = cwd;
    pane_state.entries = scan_dir(cwd);
    pane_state.cursor = 0;
    pane_state.bookmarks = &bookmarks;

    auto screen = ScreenInteractive::Fullscreen();

    pane_state.open_callback = [&](const std::string &path) {
        screen.WithRestoredIO([path] {
            fs_ops::open_in_editor(path);
        })();
    };

    auto file_pane = make_file_pane(pane_state, theme, cfg.key_map);

    auto main_component = Renderer(file_pane, [&] {
        const DirEntry *selected = nullptr;
        std::string right_info;

        if (!pane_state.entries.empty()) {
            selected = &pane_state.entries[pane_state.cursor];
            right_info = selected->perms;
            if (selected->kind != EntryKind::Directory)
                right_info += "  " + fmt_size(selected->size);
        }

        auto status_bar = hbox({
            text("  " + pane_state.cwd + "  ") | color(theme.status_fg) | bold,
            filler(),
            text(pane_state.status_msg + "  ") | color(theme.symlink),
            filler(),
            text(right_info + "  ") | color(theme.status_fg),
        }) | bgcolor(theme.status_bg);

        auto body = hbox({
            file_pane->Render() | flex,
            separator() | color(theme.border),
            render_preview(selected, theme) | flex,
        });

        return vbox({
            body | flex,
            separator() | color(theme.border),
            status_bar,
        });
    });

    main_component = CatchEvent(main_component, [&](Event event) {
        if (resolve_action(event, cfg.key_map) == Action::Quit) {
            screen.ExitLoopClosure()();
            return true;
        }
        return false;
    });

    screen.Loop(main_component);

    if (!cd_file.empty()) {
        std::ofstream f(cd_file);
        if (f) f << pane_state.cwd << '\n';
    }
}
