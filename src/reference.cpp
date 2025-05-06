#include "motor/reference.h"
#include <fstream>
#include <sstream>

namespace motor {

Reference::Reference(const fs::path& refsDir) : refsDir(refsDir) {}

void Reference::update(const std::string& refName, const Hash& hash) {
    fs::path refPath = getRefPath(refName);
    
    fs::create_directories(refPath.parent_path());
    
    std::ofstream file(refPath);
    file << hash;
    file.close();
}

Hash Reference::read(const std::string& refName) {
    fs::path refPath = getRefPath(refName);
    
    if (!fs::exists(refPath)) {
        throw std::runtime_error("Reference not found: " + refName);
    }
    
    std::ifstream file(refPath);
    std::string hash;
    std::getline(file, hash);
    file.close();
    
    if (hash.empty()) {
        throw std::runtime_error("Empty reference: " + refName);
    }
    
    if (hash.substr(0, 4) == "ref:") {
        std::string target = hash.substr(5);
        return read(target);
    }
    
    return hash;
}

void Reference::remove(const std::string& refName) {
    fs::path refPath = getRefPath(refName);
    
    if (fs::exists(refPath)) {
        fs::remove(refPath);
    }
}

std::vector<std::string> Reference::list(const std::string& prefix) const {
    std::vector<std::string> refs;
    
    fs::path prefixPath = refsDir / prefix;
    if (!fs::exists(prefixPath)) {
        return refs;
    }
    
    for (const auto& entry : fs::recursive_directory_iterator(prefixPath)) {
        if (fs::is_regular_file(entry.path())) {
            std::string fullPath = entry.path().string();
            std::string relativePath = fullPath.substr(refsDir.string().length() + 1);
            
            if (prefix.empty() || relativePath.substr(0, prefix.length()) == prefix) {
                if (!prefix.empty()) {
                    relativePath = relativePath.substr(prefix.length());
                }
                
                refs.push_back(relativePath);
            }
        }
    }
    
    return refs;
}

std::string Reference::getCurrentBranch() const {
    fs::path headPath = refsDir / "HEAD";
    
    if (!fs::exists(headPath)) {
        return "";
    }
    
    std::ifstream file(headPath);
    std::string content;
    std::getline(file, content);
    file.close();
    
    const std::string prefix = "ref: refs/heads/";
    if (content.substr(0, prefix.length()) == prefix) {
        return content.substr(prefix.length());
    }
    
    return "";
}

void Reference::setHead(const std::string& refPath) {
    fs::path headPath = refsDir / "HEAD";
    
    std::ofstream file(headPath);
    file << "ref: " << refPath;
    file.close();
}

void Reference::setDetachedHead(const Hash& commitHash) {
    fs::path headPath = refsDir / "HEAD";
    
    std::ofstream file(headPath);
    file << commitHash;
    file.close();
}

fs::path Reference::getRefPath(const std::string& refName) const {
    return refsDir / refName;
}

} // namespace motor 