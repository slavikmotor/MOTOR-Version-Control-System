#include "motor/objects/tag.h"
#include <sstream>

namespace motor {

Tag::Tag(const std::string& name, const Hash& objectHash, const std::string& message)
    : name(name), objectHash(objectHash), message(message) {
    timestamp = std::time(nullptr);
    tagger = "Motor <motor@example.com>";
}

ObjectType Tag::getType() const {
    return ObjectType::TAG;
}

std::string Tag::serialize() const {
    std::ostringstream content;
    
    content << "object " << objectHash << "\n";
    content << "type commit\n";
    content << "tag " << name << "\n";
    content << "tagger " << tagger << " " << timestamp << " +0000\n";
    
    if (!message.empty()) {
        content << "\n" << message << "\n";
    }
    
    return content.str();
}

std::string Tag::getName() const {
    return name;
}

Hash Tag::getObjectHash() const {
    return objectHash;
}

std::string Tag::getMessage() const {
    return message;
}

std::time_t Tag::getTimestamp() const {
    return timestamp;
}

std::unique_ptr<Tag> Tag::deserialize(const std::string& data) {
    std::istringstream stream(data);
    std::string line;
    
    Hash objectHash;
    std::string name;
    std::string message;
    std::string tagger;
    
    while (std::getline(stream, line)) {
        if (line.empty()) {
            std::string restOfData;
            std::getline(stream, restOfData, '\0');
            message = line + "\n" + restOfData;
            break;
        }
        
        if (line.substr(0, 7) == "object ") {
            objectHash = line.substr(7);
        } else if (line.substr(0, 4) == "tag ") {
            name = line.substr(4);
        } else if (line.substr(0, 7) == "tagger ") {
            tagger = line.substr(7);
        }
    }
    
    if (objectHash.empty() || name.empty()) {
        throw std::runtime_error("Invalid tag format");
    }
    
    return std::make_unique<Tag>(name, objectHash, message);
}

} // namespace motor 