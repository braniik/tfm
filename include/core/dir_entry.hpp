#pragma once

#include <string>
#include <vector>
#include <cstdint>

enum class EntryKind : uint8_t {
    Directory,
    HiddenDir,
    File,
    Executable,
    Symlink,
    Hidden,
    Unknown
};

struct DirEntry {
    std::string name;
    std::string path;
    EntryKind kind;
    uintmax_t size;
    std::string perms;
    char git_status = 0;
};

std::vector<DirEntry> scan_dir(const std::string& path);
std::string fmt_size(uintmax_t bytes);
