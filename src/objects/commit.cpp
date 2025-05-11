#include "motor/objects/commit.h"
#include "motor/utils.h"
#include <sstream>
#include <iomanip>

namespace motor {

Commit::Commit(const Hash& treeHash, const std::string& message)
    : treeHash(treeHash), message(message) {
    timestamp = std::time(nullptr);
    author = "Motor <motor@example.com>";
    committer = "Motor <motor@example.com>";
}

ObjectType Commit::getType() const {
    return ObjectType::COMMIT;
}

std::string Commit::serialize() const {
    std::ostringstream content;
    
    content << "tree " << treeHash << "\n";
    
    if (!parentHash.empty()) {
        content << "parent " << parentHash << "\n";
    }
    
    content << "author " << author << " " << timestamp << " +0000\n";
    content << "committer " << committer << " " << timestamp << " +0000\n";
    content << "\n" << message << "\n";
    
    return content.str();
}

void Commit::setParent(const Hash& parentHash) {
    this->parentHash = parentHash;
    hash.clear();
}

Hash Commit::getParent() const {
    return parentHash;
}

Hash Commit::getTreeHash() const {
    return treeHash;
}

std::string Commit::getMessage() const {
    return message;
}

std::time_t Commit::getTimestamp() const {
    return timestamp;
}

std::unique_ptr<Commit> Commit::deserialize(const std::string& data) {
    std::istringstream stream(data);
    std::string line;
    
    Hash treeHash;
    Hash parentHash;
    std::string message;
    std::string author;
    std::string committer;
    std::time_t timestamp = 0;
    
    while (std::getline(stream, line)) {
        if (line.empty()) {
            std::string restOfData;
            std::getline(stream, restOfData);
            message = restOfData;
            break;
        }
        
        if (line.substr(0, 5) == "tree ") {
            treeHash = line.substr(5);
        } else if (line.substr(0, 7) == "parent ") {
            parentHash = line.substr(7);
        } else if (line.substr(0, 7) == "author ") {
            author = line.substr(7);
            size_t timestampPos = author.rfind(' ');
            if (timestampPos != std::string::npos) {
                size_t tzPos = author.rfind(' ', timestampPos - 1);
                if (tzPos != std::string::npos) {
                    std::string timestampStr = author.substr(tzPos + 1, timestampPos - tzPos - 1);
                    try {
                        timestamp = std::stol(timestampStr);
                    } catch (...) {}
                    author = author.substr(0, tzPos);
                }
            }
        } else if (line.substr(0, 10) == "committer ") {
            committer = line.substr(10);
        }
    }
    
    if (treeHash.empty()) {
        throw std::runtime_error("Invalid commit format: missing tree hash");
    }
    
    auto commit = std::make_unique<Commit>(treeHash, message);
    if (!parentHash.empty()) {
        commit->setParent(parentHash);
    }
    
    return commit;
}

} // namespace motor 