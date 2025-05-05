#ifndef MOTOR_REFERENCE_H
#define MOTOR_REFERENCE_H

#include "motor/objects/object.h"
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace motor {

class Reference {
public:
    Reference(const fs::path& refsDir);
    
    void update(const std::string& refName, const Hash& hash);
    Hash read(const std::string& refName);
    void remove(const std::string& refName);
    std::vector<std::string> list(const std::string& prefix = "") const;
    
    std::string getCurrentBranch() const;
    void setHead(const std::string& refPath);
    void setDetachedHead(const Hash& commitHash);

private:
    fs::path refsDir;
    fs::path getRefPath(const std::string& refName) const;
};

} // namespace motor

#endif // MOTOR_REFERENCE_H 