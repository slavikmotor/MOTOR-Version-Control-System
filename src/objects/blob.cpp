#include "motor/objects/blob.h"

namespace motor {

Blob::Blob(const std::string& content) : content(content) {}

ObjectType Blob::getType() const {
    return ObjectType::BLOB;
}

std::string Blob::serialize() const {
    return content;
}

const std::string& Blob::getContent() const {
    return content;
}

std::unique_ptr<Blob> Blob::deserialize(const std::string& data) {
    return std::make_unique<Blob>(data);
}

} // namespace motor 