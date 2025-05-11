#include "motor/repository.h"
#include "motor/index.h"
#include "motor/reference.h"
#include "motor/utils.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>

namespace motor {

Repository::Repository(const std::string& path) {
    repoPath = fs::absolute(path);
    motorDir = repoPath / ".motor";
    
    if (!fs::exists(motorDir) || !fs::is_directory(motorDir)) {
        throw std::runtime_error("Not a valid motor repository: " + path);
    }
}

Repository::~Repository() {}

Repository Repository::init(const std::string& path) {
    fs::path repoPath = fs::absolute(path);
    fs::path motorDir = repoPath / ".motor";
    
    if (!fs::exists(repoPath)) {
        fs::create_directories(repoPath);
    } else if (!fs::is_directory(repoPath)) {
        throw std::runtime_error("Path exists but is not a directory: " + repoPath.string());
    }
    
    fs::create_directory(motorDir);
    fs::create_directory(motorDir / "objects");
    fs::create_directory(motorDir / "refs");
    fs::create_directory(motorDir / "refs/heads");
    fs::create_directory(motorDir / "refs/tags");
    
    std::ofstream headFile(motorDir / "HEAD");
    headFile << "ref: refs/heads/master";
    headFile.close();
    
    std::ofstream indexFile(motorDir / "index");
    indexFile.close();
    
    return Repository(path);
}

fs::path Repository::getPath() const {
    return repoPath;
}

fs::path Repository::getMotorDir() const {
    return motorDir;
}

Hash Repository::hashData(const std::string& data) {
    return utils::hashSHA1(data);
}

fs::path Repository::getObjectPath(const Hash& hash) {
    return motorDir / "objects" / hash.substr(0, 2) / hash.substr(2);
}

std::string Repository::compressData(const std::string& data) {
    return utils::compressData(data);
}

std::string Repository::decompressData(const std::string& data) {
    return utils::decompressData(data);
}

Hash Repository::writeObject(std::unique_ptr<Object> object) {
    ObjectType type = object->getType();
    std::string data = object->serialize();
    Hash hash = object->getHash();
    
    fs::path objPath = getObjectPath(hash);
    
    if (!fs::exists(objPath)) {
        std::string header = typeToString(type) + " " + std::to_string(data.size()) + "\0";
        std::string store = header + data;
        
        std::string compressed = compressData(store);
        
        fs::create_directories(objPath.parent_path());
        std::ofstream file(objPath, std::ios::binary);
        file.write(compressed.data(), compressed.size());
        file.close();
    }
    
    return hash;
}

std::unique_ptr<Object> Repository::readObject(const Hash& hash) {
    fs::path objPath = getObjectPath(hash);
    
    if (!fs::exists(objPath)) {
        throw std::runtime_error("Object not found: " + hash);
    }
    
    std::ifstream file(objPath, std::ios::binary);
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    std::string decompressed = decompressData(buffer.str());
    
    size_t nullPos = decompressed.find('\0');
    if (nullPos == std::string::npos) {
        throw std::runtime_error("Invalid object format");
    }
    
    std::string header = decompressed.substr(0, nullPos);
    size_t spacePos = header.find(' ');
    if (spacePos == std::string::npos) {
        throw std::runtime_error("Invalid object header format");
    }
    
    std::string typeStr = header.substr(0, spacePos);
    ObjectType type = stringToType(typeStr);
    
    std::string data = decompressed.substr(nullPos + 1);
    
    return Object::deserialize(data, type);
}

void Repository::updateRef(const std::string& refName, const Hash& hash) {
    Reference refs(motorDir);
    refs.update(refName, hash);
}

Hash Repository::readRef(const std::string& refName) {
    Reference refs(motorDir);
    return refs.read(refName);
}

std::vector<std::string> Repository::getAllRefs() {
    Reference refs(motorDir);
    return refs.list();
}

void Repository::createBranch(const std::string& name, const Hash& commitHash) {
    auto object = readObject(commitHash);
    if (object->getType() != ObjectType::COMMIT) {
        throw std::runtime_error("Not a commit object: " + commitHash);
    }
    
    std::string branchPath = "refs/heads/" + name;
    updateRef(branchPath, commitHash);
}

void Repository::deleteBranch(const std::string& name) {
    std::string currentBranch = getCurrentBranch();
    if (currentBranch == name) {
        throw std::runtime_error("Cannot delete the current branch");
    }
    
    std::string branchPath = "refs/heads/" + name;
    fs::path refPath = motorDir / branchPath;
    
    if (!fs::exists(refPath)) {
        throw std::runtime_error("Branch not found: " + name);
    }
    
    fs::remove(refPath);
}

std::vector<std::string> Repository::listBranches() {
    Reference refs(motorDir);
    return refs.list("refs/heads/");
}

std::string Repository::getCurrentBranch() {
    Reference refs(motorDir);
    return refs.getCurrentBranch();
}

void Repository::createTag(const std::string& name, const Hash& commitHash, const std::string& message) {
    auto object = readObject(commitHash);
    if (object->getType() != ObjectType::COMMIT) {
        throw std::runtime_error("Not a commit object: " + commitHash);
    }
    
    std::string tagPath = "refs/tags/" + name;
    
    if (message.empty()) {
        updateRef(tagPath, commitHash);
    } else {
        auto tag = std::make_unique<Tag>(name, commitHash, message);
        Hash tagHash = writeObject(std::move(tag));
        updateRef(tagPath, tagHash);
    }
}

void Repository::deleteTag(const std::string& name) {
    std::string tagPath = "refs/tags/" + name;
    fs::path refPath = motorDir / tagPath;
    
    if (!fs::exists(refPath)) {
        throw std::runtime_error("Tag not found: " + name);
    }
    
    fs::remove(refPath);
}

std::vector<std::string> Repository::listTags() {
    Reference refs(motorDir);
    return refs.list("refs/tags/");
}

Hash Repository::writeTree() {
    Index index(motorDir / "index");
    index.load();
    
    auto tree = std::make_unique<Tree>();
    
    for (const auto& entry : index.getEntries()) {
        tree->addEntry(TreeEntry(fs::path(entry.getPath()).filename().string(), entry.getHash(), entry.getMode()));
    }
    
    return writeObject(std::move(tree));
}

Hash Repository::commit(const std::string& message) {
    Hash treeHash = writeTree();
    
    auto commit = std::make_unique<Commit>(treeHash, message);
    
    std::string currentBranch = getCurrentBranch();
    if (!currentBranch.empty()) {
        try {
            Hash parentHash = readRef("refs/heads/" + currentBranch);
            commit->setParent(parentHash);
        } catch (const std::exception&) {
        }
    }
    
    Hash commitHash = writeObject(std::move(commit));
    
    if (!currentBranch.empty()) {
        updateRef("refs/heads/" + currentBranch, commitHash);
    }
    
    return commitHash;
}

std::vector<Hash> Repository::getCommitHistory(const Hash& startCommit) {
    std::vector<Hash> history;
    Hash current = startCommit;
    
    while (!current.empty()) {
        history.push_back(current);
        
        try {
            auto object = readObject(current);
            if (object->getType() != ObjectType::COMMIT) {
                break;
            }
            
            auto commit = static_cast<Commit*>(object.get());
            current = commit->getParent();
        } catch (const std::exception&) {
            break;
        }
    }
    
    return history;
}

void Repository::readTreeToWorkdir(const Hash& treeHash, const fs::path& path) {
    auto object = readObject(treeHash);
    if (object->getType() != ObjectType::TREE) {
        throw std::runtime_error("Not a tree object: " + treeHash);
    }
    
    auto tree = static_cast<Tree*>(object.get());
    
    if (!fs::exists(path)) {
        fs::create_directories(path);
    }
    
    for (const auto& entry : tree->getEntries()) {
        fs::path entryPath = path / entry.name;
        
        if ((entry.mode & 0040000) != 0) {
            readTreeToWorkdir(entry.hash, entryPath);
        } else {
            auto blobObject = readObject(entry.hash);
            if (blobObject->getType() != ObjectType::BLOB) {
                throw std::runtime_error("Not a blob object: " + entry.hash);
            }
            
            auto blob = static_cast<Blob*>(blobObject.get());
            
            std::ofstream file(entryPath, std::ios::binary);
            file.write(blob->getContent().data(), blob->getContent().size());
            file.close();
            
            fs::permissions(entryPath, static_cast<fs::perms>(entry.mode));
        }
    }
}

void Repository::checkout(const Hash& commitHash) {
    auto object = readObject(commitHash);
    if (object->getType() != ObjectType::COMMIT) {
        throw std::runtime_error("Not a commit object: " + commitHash);
    }
    
    auto commit = static_cast<Commit*>(object.get());
    Hash treeHash = commit->getTreeHash();
    
    for (const auto& entry : fs::directory_iterator(repoPath)) {
        if (entry.path().filename() != ".motor") {
            if (fs::is_directory(entry.path())) {
                fs::remove_all(entry.path());
            } else {
                fs::remove(entry.path());
            }
        }
    }
    
    readTreeToWorkdir(treeHash, repoPath);
    
    Reference refs(motorDir);
    refs.setDetachedHead(commitHash);
}

void Repository::checkoutBranch(const std::string& branchName) {
    std::string refPath = "refs/heads/" + branchName;
    
    try {
        Hash commitHash = readRef(refPath);
        checkout(commitHash);
        
        Reference refs(motorDir);
        refs.setHead(refPath);
    } catch (const std::exception& e) {
        throw std::runtime_error("Branch not found: " + branchName);
    }
}

void Repository::add(const std::string& path) {
    fs::path fullPath = repoPath / path;
    
    if (!fs::exists(fullPath)) {
        throw std::runtime_error("Path does not exist: " + path);
    }
    
    Index index(motorDir / "index");
    index.load();
    
    if (fs::is_regular_file(fullPath)) {
        std::ifstream file(fullPath, std::ios::binary);
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        auto blob = std::make_unique<Blob>(buffer.str());
        Hash hash = writeObject(std::move(blob));
        
        struct stat st;
        stat(fullPath.string().c_str(), &st);
        uint32_t mode = st.st_mode & 0777;
        
        if (index.contains(path)) {
            auto entry = index.findEntry(path);
            entry->setHash(hash);
            entry->setMode(mode);
            entry->setMtime(fs::last_write_time(fullPath).time_since_epoch().count());
        } else {
            index.add(IndexEntry(path, hash, mode));
        }
    } else if (fs::is_directory(fullPath)) {
        for (const auto& entry : fs::recursive_directory_iterator(fullPath)) {
            if (fs::is_regular_file(entry.path())) {
                std::string relPath = entry.path().string().substr(repoPath.string().length() + 1);
                add(relPath);
            }
        }
    }
    
    index.save();
}

void Repository::remove(const std::string& path) {
    Index index(motorDir / "index");
    index.load();
    
    index.remove(path);
    index.save();
}

std::unordered_map<std::string, Hash> Repository::getIndexEntries() {
    std::unordered_map<std::string, Hash> result;
    
    Index index(motorDir / "index");
    index.load();
    
    for (const auto& entry : index.getEntries()) {
        result[entry.getPath()] = entry.getHash();
    }
    
    return result;
}

} // namespace motor 