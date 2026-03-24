#pragma once

#include <map>
#include <string>

using BookmarkMap = std::map<char, std::string>;

BookmarkMap load_bookmarks();
void save_bookmarks(const BookmarkMap& bm);
