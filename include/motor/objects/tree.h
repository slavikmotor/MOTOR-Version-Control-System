#ifndef MOTOR_TREE_H
#define MOTOR_TREE_H

#include "motor/objects/object.h"
#include <string>
#include <vector>
#include <cstdint>

namespace motor {

struct TreeEntry {
    std::string name;
    Hash hash;
    uint32_t mode;
    
    TreeEntry(const std::string& name, const Hash& hash, uint32_t mode);
};

class Tree : public Object {
public:
    Tree();
    
    ObjectType getType() const override;
    std::string serialize() const override;
    
    void addEntry(const TreeEntry& entry);
    const std::vector<TreeEntry>& getEntries() const;
    
    static std::unique_ptr<Tree> deserialize(const std::string& data);

private:
    std::vector<TreeEntry> entries;
};

} // namespace motor

#endif // MOTOR_TREE_H 