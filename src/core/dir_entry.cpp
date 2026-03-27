#include "core/dir_entry.hpp"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

static std::string perms_string(mode_t mode) {
    char p[10] = {
        (mode & S_IRUSR) ? 'r' : '-',
        (mode & S_IWUSR) ? 'w' : '-',
        (mode & S_IXUSR) ? 'x' : '-',
        (mode & S_IRGRP) ? 'r' : '-',
        (mode & S_IWGRP) ? 'w' : '-',
        (mode & S_IXGRP) ? 'x' : '-',
        (mode & S_IROTH) ? 'r' : '-',
        (mode & S_IWOTH) ? 'w' : '-',
        (mode & S_IXOTH) ? 'x' : '-',
        '\0'
    };
    return std::string(p);
}

static EntryKind classify(const fs::path& p, const struct stat& st) {
    const std::string name = p.filename().string();
    const bool dot = !name.empty() && name[0] == '.';
    if (S_ISLNK(st.st_mode)) return EntryKind::Symlink;
    if (S_ISDIR(st.st_mode)) return dot ? EntryKind::HiddenDir : EntryKind::Directory;
    if (dot) return EntryKind::Hidden;
    if (st.st_mode & S_IXUSR) return EntryKind::Executable;
    if (S_ISREG(st.st_mode)) return EntryKind::File;
    return EntryKind::Unknown;
}

std::vector<DirEntry> scan_dir(const std::string& path) {
    std::vector<DirEntry> entries;

    if (path != "/") {
        const std::string parent = fs::path(path).parent_path().string();
        entries.push_back({ "..", parent, EntryKind::Directory, 0, "rwxr-xr-x" });
    }

    try {
        for (const auto& e : fs::directory_iterator(path, fs::directory_options::skip_permission_denied)) {
            struct stat st {};
            if (::lstat(e.path().c_str(), &st) != 0) continue;

            DirEntry de;
            de.name = e.path().filename().string();
            de.path = e.path().string();
            de.kind = classify(e.path(), st);
            de.size = S_ISREG(st.st_mode) ? static_cast<uintmax_t>(st.st_size) : 0;
            de.perms = perms_string(st.st_mode);

            entries.push_back(std::move(de));
        }
    } catch (...) {}

    const auto sort_begin = entries.begin() + (path != "/" ? 1 : 0);
    std::sort(sort_begin, entries.end(), [](const DirEntry& a, const DirEntry& b) {
        const bool a_dir = (a.kind == EntryKind::Directory || a.kind == EntryKind::HiddenDir);
        const bool b_dir = (b.kind == EntryKind::Directory || b.kind == EntryKind::HiddenDir);
        if (a_dir != b_dir) return a_dir > b_dir;
        return a.name < b.name;
    });

    {
        std::unordered_map<std::string, char> git_map;
        const std::string cmd = "git -C " + path + " status --porcelain --untracked-files=normal 2>/dev/null";
        if (FILE* pipe = ::popen(cmd.c_str(), "r")) {
            char line[512];
            while (std::fgets(line, sizeof(line), pipe)) {
                if (line[0] == '\0' || line[1] == '\0' || line[2] == '\0') continue;
                const char xy = (line[0] != ' ' && line[0] != '?') ? line[0] : line[1];
                std::string name = line + 3;
                if (!name.empty() && name.back() == '\n') name.pop_back();
                const auto slash = name.rfind('/');
                if (slash != std::string::npos) name = name.substr(slash + 1);
                if (!name.empty()) git_map[name] = xy;
            }
            ::pclose(pipe);
        }
        for (auto& e : entries) {
            auto it = git_map.find(e.name);
            if (it != git_map.end()) e.git_status = it->second;
        }
    }

    return entries;
}

std::string fmt_size(uintmax_t bytes) {
    static const char* units[] = { "B", "KB", "MB", "GB", "TB" };
    double val = static_cast<double>(bytes);
    int i = 0;
    while (val >= 1024.0 && i < 4) { val /= 1024.0; ++i; }

    std::ostringstream oss;
    if (i == 0) {
        oss << bytes << " B";
    } else {
        oss << std::fixed << std::setprecision(1) << val << " " << units[i];
    }
    return oss.str();
}
