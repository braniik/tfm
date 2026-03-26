#include "core/config.hpp"
#include "input/keybinds.hpp"
#include "ui/theme.hpp"

#include <toml++/toml.hpp>

#include <cstdlib>
#include <filesystem>
#include <string>
#include <unordered_map>

namespace fs = std::filesystem;

static fs::path config_path() {
    const char *xdg = std::getenv("XDG_CONFIG_HOME");
    fs::path cfg;
    if (xdg && xdg[0] != '\0') {
        cfg = fs::path(xdg);
    } else {
        const char *home = std::getenv("HOME");
        cfg = fs::path(home ? home : "/tmp") / ".config";
    }
    return cfg / "tfm" / "config.toml";
}

static ftxui::Color parse_hex(const std::string &s, ftxui::Color fallback) {
    if (s.size() == 7 && s[0] == '#') {
        try {
            int r = std::stoi(s.substr(1, 2), nullptr, 16);
            int g = std::stoi(s.substr(3, 2), nullptr, 16);
            int b = std::stoi(s.substr(5, 2), nullptr, 16);
            return ftxui::Color::RGB(
                static_cast<uint8_t>(r),
                static_cast<uint8_t>(g),
                static_cast<uint8_t>(b));
        } catch (...) {}
    }
    return fallback;
}

static std::unordered_map<std::string, Action> default_key_map() {
    return {
        {"k", Action::MoveUp},
        {"ArrowUp", Action::MoveUp},
        {"j", Action::MoveDown},
        {"ArrowDown", Action::MoveDown},
        {"PageUp", Action::PageUp},
        {"PageDown", Action::PageDown},
        {"l", Action::Enter},
        {"ArrowRight", Action::Enter},
        {"Enter", Action::Enter},
        {"h", Action::Back},
        {"ArrowLeft", Action::Back},
        {"Backspace", Action::Back},
        {"o", Action::Open},
        {"d", Action::Delete},
        {"r", Action::Rename},
        {"y", Action::Yank},
        {"x", Action::Cut},
        {"p", Action::Paste},
        {"n", Action::NewFile},
        {"N", Action::NewDir},
        {"/", Action::Search},
        {"m", Action::BookmarkAdd},
        {";", Action::BookmarkJump},
        {"q", Action::Quit},
    };
}

static const std::unordered_map<std::string, Action> ACTION_NAMES = {
    {"MoveUp", Action::MoveUp},
    {"MoveDown", Action::MoveDown},
    {"PageUp", Action::PageUp},
    {"PageDown", Action::PageDown},
    {"Enter", Action::Enter},
    {"Back", Action::Back},
    {"Open", Action::Open},
    {"Delete", Action::Delete},
    {"Rename", Action::Rename},
    {"Yank", Action::Yank},
    {"Cut", Action::Cut},
    {"Paste", Action::Paste},
    {"NewFile", Action::NewFile},
    {"NewDir", Action::NewDir},
    {"Search", Action::Search},
    {"BookmarkAdd", Action::BookmarkAdd},
    {"BookmarkJump", Action::BookmarkJump},
    {"Quit", Action::Quit},
};

Config load_config() {
    Config cfg;
    cfg.key_map = default_key_map();
    cfg.theme = default_theme();

    toml::table tbl;
    try {
        tbl = toml::parse_file(config_path().string());
    } catch (...) {
        return cfg;
    }

    if (const auto *keys_tbl = tbl["keys"].as_table()) {
        for (const auto &[action_name, val] : *keys_tbl) {
            auto act_it = ACTION_NAMES.find(std::string(action_name));
            if (act_it == ACTION_NAMES.end()) continue;

            const auto *arr = val.as_array();
            if (!arr) continue;

            for (auto it = cfg.key_map.begin(); it != cfg.key_map.end(); ) {
                if (it->second == act_it->second)
                    it = cfg.key_map.erase(it);
                else
                    ++it;
            }

            for (const auto &elem : *arr) {
                if (const auto *s = elem.as_string())
                    cfg.key_map[s->get()] = act_it->second;
            }
        }
    }

    if (const auto *theme_tbl = tbl["theme"].as_table()) {
        auto get = [&](const char *key, ftxui::Color fallback) {
            if (const auto *v = (*theme_tbl)[key].as_string())
                return parse_hex(v->get(), fallback);
            return fallback;
        };
        Theme &t = cfg.theme;
        t.dir = get("dir", t.dir);
        t.file = get("file", t.file);
        t.exec = get("exec", t.exec);
        t.symlink = get("symlink", t.symlink);
        t.hidden = get("hidden", t.hidden);
        t.selected_bg = get("selected_bg", t.selected_bg);
        t.selected_fg = get("selected_fg", t.selected_fg);
        t.status_fg  = get("status_fg", t.status_fg);
        t.status_bg  = get("status_bg", t.status_bg);
        t.border     = get("border", t.border);
        t.pane_title = get("pane_title", t.pane_title);
    }

    return cfg;
}
