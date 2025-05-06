#ifndef MOTOR_TAG_H
#define MOTOR_TAG_H

#include "motor/objects/object.h"
#include <string>
#include <ctime>

namespace motor {

class Tag : public Object {
public:
    Tag(const std::string& name, const Hash& objectHash, const std::string& message);
    
    ObjectType getType() const override;
    std::string serialize() const override;
    
    std::string getName() const;
    Hash getObjectHash() const;
    std::string getMessage() const;
    std::time_t getTimestamp() const;
    
    static std::unique_ptr<Tag> deserialize(const std::string& data);

private:
    std::string name;
    Hash objectHash;
    std::string message;
    std::string tagger;
    std::time_t timestamp;
};

} // namespace motor

#endif // MOTOR_TAG_H 