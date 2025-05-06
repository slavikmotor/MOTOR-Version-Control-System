#ifndef MOTOR_BLOB_H
#define MOTOR_BLOB_H

#include "motor/objects/object.h"
#include <string>

namespace motor {

class Blob : public Object {
public:
    Blob(const std::string& content);
    
    ObjectType getType() const override;
    std::string serialize() const override;
    
    const std::string& getContent() const;
    
    static std::unique_ptr<Blob> deserialize(const std::string& data);

private:
    std::string content;
};

} // namespace motor

#endif // MOTOR_BLOB_H 