#pragma once

#include <string>
#include <vector>

namespace fs_ops {
    bool delete_path(const std::string& path, std::string& err);
    bool copy_path(const std::string& src, const std::string& dst_dir, std::string& err);
    bool move_path(const std::string& src, const std::string& dst_dir, std::string& err);
    bool rename_entry(const std::string& path, const std::string& new_name, std::string& err);
    void open_in_editor(const std::string& path);
    void open_in_editor(const std::string& path, const std::string& editor);
    std::vector<std::string> find_editors();
}
