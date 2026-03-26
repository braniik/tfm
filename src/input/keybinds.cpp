#include "input/keybinds.hpp"

using namespace ftxui;

std::string event_to_key(const Event &event) {
    if (event.is_character()) return event.character();
    if (event == Event::ArrowUp) return "ArrowUp";
    if (event == Event::ArrowDown) return "ArrowDown";
    if (event == Event::ArrowLeft) return "ArrowLeft";
    if (event == Event::ArrowRight) return "ArrowRight";
    if (event == Event::Return) return "Enter";
    if (event == Event::Backspace) return "Backspace";
    if (event == Event::PageUp) return "PageUp";
    if (event == Event::PageDown) return "PageDown";
    if (event == Event::Escape) return "Escape";
    return "";
}

Action resolve_action(const Event &event,
                      const std::unordered_map<std::string, Action> &key_map) {
    const auto key = event_to_key(event);
    if (key.empty()) return Action::Unknown;
    const auto it = key_map.find(key);
    return it != key_map.end() ? it->second : Action::Unknown;
}
