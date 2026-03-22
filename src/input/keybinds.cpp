#include "input/keybinds.hpp"

using namespace ftxui;

Action resolve_action(const Event& event) {
    if (event == Event::ArrowUp || event == Event::Character('k')) return Action::MoveUp;
    if (event == Event::ArrowDown || event == Event::Character('j')) return Action::MoveDown;
    if (event == Event::PageUp) return Action::PageUp;
    if (event == Event::PageDown) return Action::PageDown;
    if (event == Event::Return || event == Event::ArrowRight || event == Event::Character('l')) return Action::Enter;
    if (event == Event::Backspace || event == Event::ArrowLeft || event == Event::Character('h')) return Action::Back;
    if (event == Event::Character('o')) return Action::Open;
    if (event == Event::Character('d')) return Action::Delete;
    if (event == Event::Character('r')) return Action::Rename;
    if (event == Event::Character('y')) return Action::Yank;
    if (event == Event::Character('x')) return Action::Cut;
    if (event == Event::Character('p')) return Action::Paste;
    if (event == Event::Character('n')) return Action::NewFile;
    if (event == Event::Character('N')) return Action::NewDir;
    if (event == Event::Character('q')) return Action::Quit;
    return Action::Unknown;
}
