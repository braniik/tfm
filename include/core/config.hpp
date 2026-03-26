#pragma once

#include "input/keybinds.hpp"
#include "ui/theme.hpp"

#include <string>
#include <unordered_map>

struct Config {
    std::unordered_map<std::string, Action> key_map;
    Theme theme;
};

Config load_config();
