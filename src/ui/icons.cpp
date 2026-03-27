#include "ui/icons.hpp"

#include <algorithm>
#include <filesystem>
#include <string>
#include <unordered_map>

namespace fs = std::filesystem;

static const std::unordered_map<std::string, const char*> EXT_ICONS = {
    // C / C++
    { ".c", " " },
    { ".h", " " },
    { ".cpp", " " },
    { ".cc", " " },
    { ".cxx", " " },
    { ".hpp", " " },
    { ".hxx", " " },

    // Scripts / interpreted
    { ".py", " " },
    { ".sh", " " },
    { ".bash", " " },
    { ".zsh", " " },
    { ".fish", " " },
    { ".rb", " " },
    { ".lua", " " },

    // Web
    { ".js", " " },
    { ".ts", " " },
    { ".jsx", " " },
    { ".tsx", " " },
    { ".html", " " },
    { ".css", " " },
    { ".scss", " " },
    { ".vue", " " },

    // Systems / compiled
    { ".rs", " " },
    { ".go", " " },
    { ".kt", " " },
    { ".kts", " " },
    { ".java", " " },
    { ".swift", " " },

    // Data / config
    { ".json", " " },
    { ".toml", " " },
    { ".yaml", " " },
    { ".yml", " " },
    { ".xml", " " },
    { ".csv", " " },
    { ".sql", " " },
    { ".env", " " },

    // Docs / text
    { ".md", " " },
    { ".txt", " " },
    { ".pdf", " " },
    { ".tex", " " },

    // Images
    { ".png", " " },
    { ".jpg", " " },
    { ".jpeg", " " },
    { ".gif", " " },
    { ".svg", " " },
    { ".webp", " " },
    { ".ico", " " },

    // Audio
    { ".mp3", " " },
    { ".flac", " " },
    { ".wav", " " },
    { ".ogg", " " },

    // Video
    { ".mp4", " " },
    { ".mkv", " " },
    { ".mov", " " },
    { ".webm", " " },

    // Archives
    { ".zip", " " },
    { ".tar", " " },
    { ".gz", " " },
    { ".xz", " " },
    { ".bz2", " " },
    { ".zst", " " },
    { ".7z", " " },
    { ".rar", " " },

    // Build / project
    { ".cmake", " " },
    { ".lock", " " },
    { ".diff", " " },
    { ".patch", " " },

    // Fonts
    { ".ttf", " " },
    { ".otf", " " },
    { ".woff", " " },
    { ".woff2", " " },
};

std::string icon_for(const DirEntry& entry) {
    switch (entry.kind) {
        case EntryKind::Directory: return " ";
        case EntryKind::HiddenDir: return " ";
        case EntryKind::Symlink: return " ";
        case EntryKind::Executable: return " ";
        case EntryKind::Hidden: {
            std::string ext = fs::path(entry.name).extension().string();
            if (!ext.empty()) {
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                auto it = EXT_ICONS.find(ext);
                if (it != EXT_ICONS.end()) return it->second;
            }
            return " ";
        }
        default: break;
    }

    std::string ext = fs::path(entry.name).extension().string();
    if (!ext.empty()) {
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        auto it = EXT_ICONS.find(ext);
        if (it != EXT_ICONS.end()) return it->second;
    }

    return " ";
}
