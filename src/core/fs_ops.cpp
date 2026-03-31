#include "core/fs_ops.hpp"

#include <cstdlib>
#include <filesystem>
#include <sstream>
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

    static void exec_editor(const std::string& editor, const std::string& path) {
        pid_t pid = ::fork();
        if (pid == 0) {
            char* args[] = {
                const_cast<char*>(editor.c_str()),
                const_cast<char*>(path.c_str()),
                nullptr
            };
            ::execvp(editor.c_str(), args);
            ::_exit(127);
        }
        if (pid > 0) {
            ::waitpid(pid, nullptr, 0);
        }
    }

    void open_in_editor(const std::string& path) {
        const char* editor = std::getenv("EDITOR");
        std::string ed = (editor && editor[0] != '\0') ? editor : "nano";
        exec_editor(ed, path);
    }

    void open_in_editor(const std::string& path, const std::string& editor) {
        exec_editor(editor, path);
    }

    std::vector<std::string> find_editors() {
        static const std::vector<std::string> known = {
            "nvim", "vim", "vi", "nano", "emacs", "hx", "micro",
            "helix", "ne", "mcedit", "joe", "kak", "kakoune", "zeditor",
        };

        const char* path_env = std::getenv("PATH");
        if (!path_env) return {};

        std::vector<std::string> path_dirs;
        std::istringstream ss(path_env);
        std::string dir;
        while (std::getline(ss, dir, ':')) {
            if (!dir.empty()) path_dirs.push_back(dir);
        }

        std::vector<std::string> result;
        for (const auto& name : known) {
            for (const auto& d : path_dirs) {
                fs::path p = fs::path(d) / name;
                std::error_code ec;
                if (fs::exists(p, ec) && ::access(p.c_str(), X_OK) == 0) {
                    result.push_back(name);
                    break;
                }
            }
        }
        return result;
    }
}
