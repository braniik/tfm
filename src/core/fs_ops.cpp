#include "core/fs_ops.hpp"

#include <cstdlib>
#include <filesystem>
#include <unistd.h>
#include <sys/wait.h>

namespace fs = std::filesystem;
namespace fs_ops {

bool delete_path(const std::string& path, std::string& err) {
    std::error_code ec;
    fs::remove_all(path, ec);
    if (ec) { err = ec.message(); return false; }
    return true;
}

bool copy_path(const std::string& src, const std::string& dst_dir, std::string& err) {
    std::error_code ec;
    const fs::path dst = fs::path(dst_dir) / fs::path(src).filename();
    fs::copy(src, dst, fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
    if (ec) { err = ec.message(); return false; }
    return true;
}

bool move_path(const std::string& src, const std::string& dst_dir, std::string& err) {
    std::error_code ec;
    const fs::path dst = fs::path(dst_dir) / fs::path(src).filename();
    fs::rename(src, dst, ec);
    if (ec) {
        fs::copy(src, dst, fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
        if (ec) { err = ec.message(); return false; }
        fs::remove_all(src, ec);
        if (ec) { err = ec.message(); return false; }
    }
    return true;
}

bool rename_entry(const std::string& path, const std::string& new_name, std::string& err) {
    std::error_code ec;
    const fs::path dst = fs::path(path).parent_path() / new_name;
    fs::rename(path, dst, ec);
    if (ec) { err = ec.message(); return false; }
    return true;
}

void open_in_editor(const std::string& path) {
    const char* editor = std::getenv("EDITOR");
    if (!editor || editor[0] == '\0') editor = "nano";

    pid_t pid = ::fork();
    if (pid == 0) {
        char* args[] = {
            const_cast<char*>(editor),
            const_cast<char*>(path.c_str()),
            nullptr
        };
        ::execvp(editor, args);
        ::_exit(127);
    }
    if (pid > 0) {
        ::waitpid(pid, nullptr, 0);
    }
}

}
