#pragma once

#include <ftxui/dom/elements.hpp>
#include "core/dir_entry.hpp"
#include "ui/theme.hpp"

ftxui::Element render_preview(const DirEntry* selected, const Theme& theme);
