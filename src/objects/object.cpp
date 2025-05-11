#include "motor/objects/object.h"
#include "motor/objects/blob.h"
#include "motor/objects/tree.h"
#include "motor/objects/commit.h"
#include "motor/objects/tag.h"
#include "motor/utils.h"
#include <stdexcept>

namespace motor {

std::string typeToString(ObjectType type) {
    switch (type) {
        case ObjectType::BLOB: return "blob";
        case ObjectType::TREE: return "tree";
        case ObjectType::COMMIT: return "commit";
        case ObjectType::TAG: return "tag";
        default: return "unknown";
    }
}

ObjectType stringToType(const std::string& typeStr) {
    if (typeStr == "blob") return ObjectType::BLOB;
    if (typeStr == "tree") return ObjectType::TREE;
    if (typeStr == "commit") return ObjectType::COMMIT;
    if (typeStr == "tag") return ObjectType::TAG;
    throw std::runtime_error("Unknown object type: " + typeStr);
}

Hash Object::getHash() const {
    if (hash.empty()) {
        std::string content = typeToString(getType()) + " " + serialize();
        hash = utils::hashSHA1(content);
    }
    return hash;
}

std::unique_ptr<Object> Object::deserialize(const std::string& data, ObjectType type) {
    switch (type) {
        case ObjectType::BLOB:
            return Blob::deserialize(data);
        case ObjectType::TREE:
            return Tree::deserialize(data);
        case ObjectType::COMMIT:
            return Commit::deserialize(data);
        case ObjectType::TAG:
            return Tag::deserialize(data);
        default:
            throw std::runtime_error("Unknown object type for deserialization");
    }
}

} // namespace motor 