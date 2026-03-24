#pragma once

#include <ftxui/component/event.hpp>

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
    Quit,
    Unknown
};

Action resolve_action(const ftxui::Event& event);
