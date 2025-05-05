#include "motor/index.h"
#include "motor/utils.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace motor {

IndexEntry::IndexEntry(const std::string& path, const Hash& hash, uint32_t mode)
    : path(path), hash(hash), mode(mode), mtime(std::time(nullptr)) {}

std::string IndexEntry::getPath() const {
    return path;
}

Hash IndexEntry::getHash() const {
    return hash;
}

uint32_t IndexEntry::getMode() const {
    return mode;
}

std::time_t IndexEntry::getMtime() const {
    return mtime;
}

void IndexEntry::setHash(const Hash& hash) {
    this->hash = hash;
}

void IndexEntry::setMtime(std::time_t mtime) {
    this->mtime = mtime;
}

void IndexEntry::setMode(uint32_t mode) {
    this->mode = mode;
}

std::string IndexEntry::toString() const {
    std::ostringstream oss;
    oss << mode << " " << hash << " " << path;
    return oss.str();
}

IndexEntry IndexEntry::fromString(const std::string& str) {
    std::istringstream iss(str);
    
    uint32_t mode;
    Hash hash;
    std::string path;
    
    iss >> mode >> hash;
    std::getline(iss >> std::ws, path);
    
    return IndexEntry(path, hash, mode);
}

Index::Index(const fs::path& indexPath) : indexPath(indexPath) {}

void Index::load() {
    entries.clear();
    
    if (!fs::exists(indexPath)) {
        return;
    }
    
    std::ifstream file(indexPath);
    std::string line;
    
    while (std::getline(file, line)) {
        if (!line.empty()) {
            entries.push_back(IndexEntry::fromString(line));
        }
    }
    
    file.close();
}

void Index::save() {
    std::ofstream file(indexPath);
    
    for (const auto& entry : entries) {
        file << entry.toString() << "\n";
    }
    
    file.close();
}

void Index::add(const IndexEntry& entry) {
    auto it = std::find_if(entries.begin(), entries.end(),
        [&entry](const IndexEntry& e) { return e.getPath() == entry.getPath(); });
    
    if (it != entries.end()) {
        *it = entry;
    } else {
        entries.push_back(entry);
    }
}

void Index::remove(const std::string& path) {
    entries.erase(
        std::remove_if(entries.begin(), entries.end(),
            [&path](const IndexEntry& entry) {
                return entry.getPath() == path || 
                      (entry.getPath().size() > path.size() && 
                       entry.getPath().substr(0, path.size() + 1) == path + "/");
            }),
        entries.end()
    );
}

bool Index::contains(const std::string& path) const {
    return std::find_if(entries.begin(), entries.end(),
        [&path](const IndexEntry& entry) { return entry.getPath() == path; }) != entries.end();
}

std::vector<IndexEntry> Index::getEntries() const {
    return entries;
}

IndexEntry* Index::findEntry(const std::string& path) {
    auto it = std::find_if(entries.begin(), entries.end(),
        [&path](const IndexEntry& entry) { return entry.getPath() == path; });
    
    if (it != entries.end()) {
        return &(*it);
    }
    
    return nullptr;
}

} // namespace motor 