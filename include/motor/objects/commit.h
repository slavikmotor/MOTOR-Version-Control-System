#ifndef MOTOR_COMMIT_H
#define MOTOR_COMMIT_H

#include "motor/objects/object.h"
#include <string>
#include <vector>
#include <ctime>

namespace motor {

class Commit : public Object {
public:
    Commit(const Hash& treeHash, const std::string& message);
    
    ObjectType getType() const override;
    std::string serialize() const override;
    
    void setParent(const Hash& parentHash);
    Hash getParent() const;
    Hash getTreeHash() const;
    std::string getMessage() const;
    std::time_t getTimestamp() const;
    
    static std::unique_ptr<Commit> deserialize(const std::string& data);

private:
    Hash treeHash;
    Hash parentHash;
    std::string message;
    std::string author;
    std::string committer;
    std::time_t timestamp;
};

} // namespace motor

#endif // MOTOR_COMMIT_H 