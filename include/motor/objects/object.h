#ifndef MOTOR_OBJECT_H
#define MOTOR_OBJECT_H

#include <string>
#include <memory>

namespace motor {

using Hash = std::string;

enum class ObjectType {
    BLOB,
    TREE,
    COMMIT,
    TAG
};

std::string typeToString(ObjectType type);
ObjectType stringToType(const std::string& typeStr);

class Object {
public:
    virtual ~Object() = default;
    
    virtual ObjectType getType() const = 0;
    virtual std::string serialize() const = 0;
    
    Hash getHash() const;
    
    static std::unique_ptr<Object> deserialize(const std::string& data, ObjectType type);

protected:
    mutable Hash hash;
};

} // namespace motor

#endif // MOTOR_OBJECT_H 