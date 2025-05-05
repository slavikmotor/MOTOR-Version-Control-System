#ifndef MOTOR_INDEX_H
#define MOTOR_INDEX_H

#include "motor/objects/object.h"
#include <string>
#include <vector>
#include <ctime>
#include <filesystem>

namespace fs = std::filesystem;

namespace motor {

class IndexEntry {
public:
    IndexEntry(const std::string& path, const Hash& hash, uint32_t mode);
    
    std::string getPath() const;
    Hash getHash() const;
    uint32_t getMode() const;
    std::time_t getMtime() const;
    
    void setHash(const Hash& hash);
    void setMtime(std::time_t mtime);
    void setMode(uint32_t mode);
    
    std::string toString() const;
    static IndexEntry fromString(const std::string& str);

private:
    std::string path;
    Hash hash;
    uint32_t mode;
    std::time_t mtime;
};

class Index {
public:
    Index(const fs::path& indexPath);
    
    void load();
    void save();
    
    void add(const IndexEntry& entry);
    void remove(const std::string& path);
    bool contains(const std::string& path) const;
    std::vector<IndexEntry> getEntries() const;
    
    IndexEntry* findEntry(const std::string& path);

private:
    fs::path indexPath;
    std::vector<IndexEntry> entries;
};

} // namespace motor

#endif // MOTOR_INDEX_H 