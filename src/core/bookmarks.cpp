#include "core/bookmarks.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

static fs::path bookmarks_path() {
    const char *xdg = std::getenv("XDG_CONFIG_HOME");
    fs::path cfg;
    if (xdg && xdg[0] != '\0') {
        cfg = fs::path(xdg);
    } else {
        const char *home = std::getenv("HOME");
        cfg = fs::path(home ? home : "/tmp") / ".config";
    }
    return cfg / "tfm" / "bookmarks";
}

BookmarkMap load_bookmarks() {
    BookmarkMap  bm;
    std::ifstream f(bookmarks_path());
    if (!f)
        return bm;
    std::string line;
    while (std::getline(f, line)) {
        if (line.size() >= 3 && line[1] == '=')
            bm[line[0]] = line.substr(2);
    }
    return bm;
}

void save_bookmarks(const BookmarkMap &bm) {
    const auto path = bookmarks_path();
    std::error_code ec;
    fs::create_directories(path.parent_path(), ec);
    std::ofstream f(path);
    if (!f)
        return;
    for (const auto &[k, v] : bm)
        f << k << '=' << v << '\n';
}
