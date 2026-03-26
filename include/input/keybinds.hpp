#pragma once

#include <ftxui/component/event.hpp>
#include <string>
#include <unordered_map>

enum class Action {
    MoveUp,
    MoveDown,
    PageUp,
    PageDown,
    Enter,
    Back,
    Open,
    Delete,
    Rename,
    Yank,
    Cut,
    Paste,
    NewFile,
    NewDir,
    Search,
    BookmarkAdd,
    BookmarkJump,
    Quit,
    Unknown
};

std::string event_to_key(const ftxui::Event &event);

Action resolve_action(const ftxui::Event &event, const std::unordered_map<std::string, Action> &key_map);
