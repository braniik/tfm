#include "ui/preview_pane.hpp"
#include "ui/icons.hpp"
#include "core/dir_entry.hpp"

#include <ftxui/dom/elements.hpp>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <taglib/audioproperties.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>

using namespace ftxui;
namespace fs = std::filesystem;

static std::string ext_of(const std::string& name) {
    auto p = name.rfind('.');
    if (p == std::string::npos) return "";
    std::string ext = name.substr(p + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

static Element kv_row(const std::string& label, const std::string& val, const Theme& t, int label_width = 14) {
    return hbox({
        text("  " + label + "  ") | color(t.hidden) | size(WIDTH, EQUAL, label_width),
        text(val) | color(t.file),
    });
}

enum class PreviewKind { Text, Image, Audio, Binary, Directory };

static bool is_text_ext(const std::string& ext) {
    static const std::vector<std::string> TEXT = {
        "txt","md","rst","csv","json","yaml","yml","toml","xml",
        "html","htm","css","js","ts","jsx","tsx",
        "cpp","cxx","cc","c","h","hpp","hxx",
        "py","rb","go","rs","java","kt","swift",
        "sh","bash","zsh","fish","lua","vim","el","lisp",
        "hs","ml","r","sql","ini","cfg","conf",
        "log","diff","patch","gitignore","cmake","mk","makefile",
    };
    return std::find(TEXT.begin(), TEXT.end(), ext) != TEXT.end();
}

static bool is_image_ext(const std::string& ext) {
    static const std::vector<std::string> IMG = {
        "png","jpg","jpeg","gif","bmp","webp","tiff","tif","ico","svg",
    };
    return std::find(IMG.begin(), IMG.end(), ext) != IMG.end();
}

static bool is_audio_ext(const std::string& ext) {
    static const std::vector<std::string> AUD = {
        "mp3","flac","ogg","opus","m4a","aac","wav","wv","ape","mpc",
    };
    return std::find(AUD.begin(), AUD.end(), ext) != AUD.end();
}

static PreviewKind classify(const DirEntry& e) {
    if (e.kind == EntryKind::Directory) return PreviewKind::Directory;
    const std::string ext = ext_of(e.name);
    if (is_text_ext(ext)) return PreviewKind::Text;
    if (is_image_ext(ext)) return PreviewKind::Image;
    if (is_audio_ext(ext)) return PreviewKind::Audio;
    return PreviewKind::Binary;
}

static constexpr int MAX_PREVIEW_LINES = 200;
static constexpr int PANE_INNER_WIDTH = 36;

static Elements text_preview_rows(const std::string& path) {
    std::ifstream f(path);
    if (!f) return {text("  (cannot open)") | dim};

    Elements rows;
    std::string line;
    int count = 0;

    while (std::getline(f, line) && count < MAX_PREVIEW_LINES) {
        if (!line.empty() && line.back() == '\r') line.pop_back();

        std::string clean;
        clean.reserve(line.size());
        for (char c : line)
            clean += (c == '\t') ? "  " : std::string(1, c);

        if ((int)clean.size() > PANE_INNER_WIDTH)
            clean = clean.substr(0, PANE_INNER_WIDTH - 1) + "\xe2\x80\xa6";

        rows.push_back(text(" " + clean));
        ++count;
    }

    if (!f.eof())
        rows.push_back(text("  \xe2\x80\xa6") | dim);

    if (rows.empty())
        rows.push_back(text("  (empty)") | dim);

    return rows;
}

struct ImageInfo {
    std::string format;
    uint32_t width = 0;
    uint32_t height = 0;
};

static std::optional<ImageInfo> probe_image(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return std::nullopt;

    uint8_t buf[26] = {};
    f.read(reinterpret_cast<char*>(buf), sizeof(buf));
    const auto n = static_cast<size_t>(f.gcount());

    if (n >= 24 && buf[0]==0x89 && buf[1]=='P' && buf[2]=='N' && buf[3]=='G') {
        uint32_t w = (uint32_t(buf[16])<<24)|(uint32_t(buf[17])<<16)|(uint32_t(buf[18])<<8)|buf[19];
        uint32_t h = (uint32_t(buf[20])<<24)|(uint32_t(buf[21])<<16)|(uint32_t(buf[22])<<8)|buf[23];
        return ImageInfo{"PNG", w, h};
    }

    if (n >= 3 && buf[0]==0xFF && buf[1]==0xD8 && buf[2]==0xFF) {
        f.seekg(2);
        uint8_t m[2];
        for (int guard = 0; guard < 64 && f.read(reinterpret_cast<char*>(m), 2); ++guard) {
            if (m[0] != 0xFF) break;
            if (m[1] == 0xC0 || m[1] == 0xC2) {
                uint8_t sof[7];
                if (f.read(reinterpret_cast<char*>(sof), 7)) {
                    uint32_t h = (uint32_t(sof[3])<<8)|sof[4];
                    uint32_t w = (uint32_t(sof[5])<<8)|sof[6];
                    return ImageInfo{"JPEG", w, h};
                }
                break;
            }
            uint8_t lb[2];
            if (!f.read(reinterpret_cast<char*>(lb), 2)) break;
            uint16_t len = (uint16_t(lb[0])<<8)|lb[1];
            if (len < 2) break;
            f.seekg(len - 2, std::ios::cur);
        }
        return ImageInfo{"JPEG"};
    }

    if (n >= 10 && buf[0]=='G' && buf[1]=='I' && buf[2]=='F') {
        uint32_t w = uint32_t(buf[6])|(uint32_t(buf[7])<<8);
        uint32_t h = uint32_t(buf[8])|(uint32_t(buf[9])<<8);
        return ImageInfo{"GIF", w, h};
    }

    if (n>=12 && buf[0]=='R' && buf[1]=='I' && buf[2]=='F' && buf[3]=='F' && buf[8]=='W' && buf[9]=='E' && buf[10]=='B' && buf[11]=='P')
        return ImageInfo{"WebP"};

    if (n >= 26 && buf[0]=='B' && buf[1]=='M') {
        uint32_t w, h;
        std::memcpy(&w, buf+18, 4);
        std::memcpy(&h, buf+22, 4);
        return ImageInfo{"BMP", w, h};
    }

    return std::nullopt;
}

static Elements image_preview_rows(const DirEntry& e, const Theme& t) {
    Elements rows;
    auto info = probe_image(e.path);

    rows.push_back(kv_row("format", info ? info->format : "unknown", t));
    if (info && (info->width || info->height))
        rows.push_back(kv_row("dimensions", std::to_string(info->width) + " x " + std::to_string(info->height), t));
    return rows;
}

static std::string fmt_duration(int secs) {
    if (secs < 0) return "?";
    std::ostringstream ss;
    ss << secs/60 << ":" << std::setw(2) << std::setfill('0') << secs%60;
    return ss.str();
}

static Elements audio_preview_rows(const DirEntry& e, const Theme& t) {
    Elements rows;

    TagLib::FileRef ref(e.path.c_str(), true, TagLib::AudioProperties::Fast);
    if (ref.isNull()) {
        rows.push_back(text("  (unreadable)") | dim);
        return rows;
    }

    auto* tag = ref.tag();
    if (tag && !tag->isEmpty()) {
        auto ts = [](const TagLib::String& s) -> std::string {
            return s.isEmpty() ? "\xe2\x80\x94" : s.to8Bit(true);
        };
        rows.push_back(kv_row("title", ts(tag->title()), t));
        rows.push_back(kv_row("artist", ts(tag->artist()), t));
        rows.push_back(kv_row("album", ts(tag->album()), t));
        if (tag->year()) rows.push_back(kv_row("year", std::to_string(tag->year()), t));
        if (tag->track()) rows.push_back(kv_row("track", std::to_string(tag->track()), t));
    } else {
        rows.push_back(text("  (no tags)") | dim);
    }

    auto* props = ref.audioProperties();
    if (props) {
        rows.push_back(text(""));
        rows.push_back(kv_row("duration", fmt_duration(props->lengthInSeconds()), t));
        rows.push_back(kv_row("bitrate", std::to_string(props->bitrate()) + " kbps", t));
        rows.push_back(kv_row("sample rate", std::to_string(props->sampleRate()) + " Hz", t));
        rows.push_back(kv_row("channels", std::to_string(props->channels()), t));
    }

    return rows;
}

static std::string elf_machine_str(uint16_t m) {
    switch (m) {
        case 0x03: return "x86";
        case 0x3e: return "x86-64";
        case 0x28: return "ARM";
        case 0xb7: return "AArch64";
        case 0xf3: return "RISC-V";
        case 0x08: return "MIPS";
        default: { std::ostringstream ss; ss<<"0x"<<std::hex<<m; return ss.str(); }
    }
}
static std::string elf_type_str(uint16_t t) {
    switch (t) {
        case 1: return "relocatable (.o)";
        case 2: return "executable";
        case 3: return "shared object";
        case 4: return "core dump";
        default: return "unknown";
    }
}

static bool try_elf_rows(const std::string& path, const Theme& t, Elements& rows) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;

    uint8_t buf[64] = {};
    f.read(reinterpret_cast<char*>(buf), sizeof(buf));
    if (f.gcount() < 20) return false;
    if (buf[0]!=0x7f||buf[1]!='E'||buf[2]!='L'||buf[3]!='F') return false;

    const bool is64 = buf[4] == 2;
    const bool isLE = buf[5] == 1;

    auto u16 = [&](int off) -> uint16_t {
        uint16_t v; std::memcpy(&v, buf+off, 2);
        return isLE ? v : __builtin_bswap16(v);
    };
    auto u32 = [&](int off) -> uint32_t {
        uint32_t v; std::memcpy(&v, buf+off, 4);
        return isLE ? v : __builtin_bswap32(v);
    };
    auto u64 = [&](int off) -> uint64_t {
        uint64_t v; std::memcpy(&v, buf+off, 8);
        return isLE ? v : __builtin_bswap64(v);
    };

    const uint16_t e_type = u16(16);
    const uint16_t e_machine = u16(18);
    const uint64_t e_entry = is64 ? u64(24) : u32(24);

    rows.push_back(text("  ELF") | color(t.exec) | bold);
    rows.push_back(text(""));
    rows.push_back(kv_row("arch", elf_machine_str(e_machine) + (is64?" (64-bit)":" (32-bit)"), t));
    rows.push_back(kv_row("type", elf_type_str(e_type), t));
    rows.push_back(kv_row("endian", isLE ? "little" : "big", t));
    if (e_entry) {
        std::ostringstream oss; oss << "0x" << std::hex << e_entry;
        rows.push_back(kv_row("entry", oss.str(), t));
    }
    return true;
}

static Elements hex_dump_rows(const std::string& path, const Theme& t) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return {text("  (cannot read)") | dim};

    constexpr int COLS = 8;
    constexpr int MAXROWS = 16;

    std::vector<uint8_t> buf(COLS * MAXROWS);
    f.read(reinterpret_cast<char*>(buf.data()), buf.size());
    const int total = static_cast<int>(f.gcount());

    Elements rows;
    for (int row = 0; row < MAXROWS && row*COLS < total; ++row) {
        const int base = row * COLS;
        const int end = std::min(base + COLS, total);

        std::ostringstream hex_part, asc_part;
        hex_part << std::hex << std::setfill('0') << std::setw(4) << base << ": ";
        for (int i = base; i < end; ++i) {
            hex_part << std::setw(2) << int(buf[i]) << " ";
            asc_part << (buf[i]>=0x20 && buf[i]<0x7f ? char(buf[i]) : '.');
        }
        for (int i = end-base; i < COLS; ++i) hex_part << "   ";

        rows.push_back(hbox({
            text(" " + hex_part.str()) | color(t.hidden),
            text(asc_part.str()) | color(t.file),
        }));
    }
    return rows;
}

static Elements binary_preview_rows(const DirEntry& e, const Theme& t) {
    Elements rows;
    const bool is_elf = try_elf_rows(e.path, t, rows);
    if (is_elf) {
        rows.push_back(text(""));
        rows.push_back(separatorLight() | color(t.border));
        rows.push_back(text("  hex") | color(t.pane_title) | bold);
        rows.push_back(text(""));
    }
    for (auto& r : hex_dump_rows(e.path, t))
        rows.push_back(std::move(r));
    return rows;
}

static Elements dir_preview_rows(const DirEntry& e, const Theme& t) {
    std::error_code ec;
    int files = 0, dirs = 0;
    uintmax_t total = 0;
    for (auto& entry : fs::directory_iterator(e.path, ec)) {
        if (ec) { ec.clear(); continue; }
        if (entry.is_directory(ec)) { ++dirs; ec.clear(); }
        else { ++files; total += entry.file_size(ec); ec.clear(); }
    }
    Elements rows;
    rows.push_back(kv_row("files", std::to_string(files), t));
    rows.push_back(kv_row("dirs", std::to_string(dirs), t));
    rows.push_back(kv_row("total", std::to_string(total), t));
    rows.push_back(text(""));
    rows.push_back(separatorLight() | color(t.border));
    rows.push_back(text(""));

    const auto entries = scan_dir(e.path);
    constexpr int MAX_SHOWN = 60;
    int shown = 0;
    for (const auto& child : entries) {
        if (shown++ >= MAX_SHOWN) {
            rows.push_back(text("  \xe2\x80\xa6") | color(t.hidden) | dim);
            break;
        }
        std::string label = " " + icon_for(child) + child.name;
        if (child.kind == EntryKind::Directory) label += "/";

        Element row = text(label);
        switch (child.kind) {
            case EntryKind::Directory: row = row | color(t.dir) | bold; break;
            case EntryKind::Executable: row = row | color(t.exec) | bold; break;
            case EntryKind::Symlink: row = row | color(t.symlink); break;
            case EntryKind::Hidden: row = row | color(t.hidden) | dim; break;
            default: row = row | color(t.file);
        }
        rows.push_back(row);
    }

    if (entries.empty())
        rows.push_back(text("  (empty)") | dim);
    return rows;
}

static const char* kind_label(EntryKind k) {
    switch (k) {
        case EntryKind::Directory: return "directory";
        case EntryKind::Executable: return "executable";
        case EntryKind::Symlink: return "symlink";
        case EntryKind::Hidden: return "hidden";
        case EntryKind::File: return "file";
        default: return "unknown";
    }
}

ftxui::Element render_preview(const DirEntry* sel, const Theme& theme) {
    if (!sel) return vbox({text("")}) | flex;

    std::string symlink_target;
    if (sel->kind == EntryKind::Symlink) {
        std::error_code ec;
        auto tgt = fs::read_symlink(sel->path, ec);
        if (!ec) symlink_target = tgt.string();
    }

    Elements rows;

    rows.push_back(text(""));
    rows.push_back(hbox({text("  " + icon_for(*sel)), text(sel->name) | bold | color(theme.file)}));
    rows.push_back(text(""));
    rows.push_back(separatorLight() | color(theme.border));
    rows.push_back(text(""));

    rows.push_back(kv_row("type", kind_label(sel->kind), theme));
    rows.push_back(kv_row("perms", sel->perms, theme));
    if (sel->kind != EntryKind::Directory)
        rows.push_back(kv_row("size", fmt_size(sel->size), theme));
    if (!symlink_target.empty())
        rows.push_back(kv_row("\xe2\x86\x92 target", symlink_target, theme));

    rows.push_back(text(""));
    rows.push_back(separatorLight() | color(theme.border));
    rows.push_back(text(""));

    switch (classify(*sel)) {
        case PreviewKind::Text:
            for (auto& r : text_preview_rows(sel->path)) rows.push_back(std::move(r));
            break;
        case PreviewKind::Image:
            for (auto& r : image_preview_rows(*sel, theme)) rows.push_back(std::move(r));
            break;
        case PreviewKind::Audio:
            for (auto& r : audio_preview_rows(*sel, theme)) rows.push_back(std::move(r));
            break;
        case PreviewKind::Binary:
            for (auto& r : binary_preview_rows(*sel, theme)) rows.push_back(std::move(r));
            break;
        case PreviewKind::Directory:
            for (auto& r : dir_preview_rows(*sel, theme)) rows.push_back(std::move(r));
            break;
    }

    rows.push_back(text(""));
    return vbox(rows) | flex;
}
