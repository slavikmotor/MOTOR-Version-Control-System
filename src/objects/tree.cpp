#include "motor/objects/tree.h"
#include "motor/utils.h"
#include <algorithm>
#include <sstream>

namespace motor {

TreeEntry::TreeEntry(const std::string& name, const Hash& hash, uint32_t mode)
    : name(name), hash(hash), mode(mode) {}

Tree::Tree() {}

ObjectType Tree::getType() const {
    return ObjectType::TREE;
}

std::string Tree::serialize() const {
    std::ostringstream content;
    
    std::vector<TreeEntry> sortedEntries = entries;
    std::sort(sortedEntries.begin(), sortedEntries.end(), 
              [](const TreeEntry& a, const TreeEntry& b) { return a.name < b.name; });
    
    for (const auto& entry : sortedEntries) {
        content << std::oct << entry.mode << " " << entry.name << '\0';
        content << utils::hexToBinary(entry.hash);
    }
    
    return content.str();
}

void Tree::addEntry(const TreeEntry& entry) {
    entries.push_back(entry);
    hash.clear();
}

const std::vector<TreeEntry>& Tree::getEntries() const {
    return entries;
}

std::unique_ptr<Tree> Tree::deserialize(const std::string& data) {
    auto tree = std::make_unique<Tree>();
    
    size_t pos = 0;
    while (pos < data.size()) {
        size_t spacePos = data.find(' ', pos);
        if (spacePos == std::string::npos) break;
        
        std::string modeStr = data.substr(pos, spacePos - pos);
        uint32_t mode = std::stoi(modeStr, nullptr, 8);
        
        size_t nullPos = data.find('\0', spacePos + 1);
        if (nullPos == std::string::npos) break;
        
        std::string name = data.substr(spacePos + 1, nullPos - spacePos - 1);
        
        if (nullPos + 21 > data.size()) break;
        
        std::string binaryHash = data.substr(nullPos + 1, 20);
        std::string hexHash = utils::binaryToHex(binaryHash);
        
        tree->addEntry(TreeEntry(name, hexHash, mode));
        
        pos = nullPos + 21;
    }
    
    return tree;
}

} // namespace motor 