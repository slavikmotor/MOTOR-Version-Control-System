#ifndef MOTOR_REPOSITORY_H
#define MOTOR_REPOSITORY_H

#include "motor/objects/object.h"
#include "motor/objects/blob.h"
#include "motor/objects/tree.h"
#include "motor/objects/commit.h"
#include "motor/objects/tag.h"
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

namespace motor {

class Repository {
public:
    Repository(const std::string& path);
    ~Repository();
    
    static Repository init(const std::string& path);
    
    fs::path getPath() const;
    fs::path getMotorDir() const;
    
    Hash writeObject(std::unique_ptr<Object> object);
    std::unique_ptr<Object> readObject(const Hash& hash);
    
    void updateRef(const std::string& refName, const Hash& hash);
    Hash readRef(const std::string& refName);
    std::vector<std::string> getAllRefs();
    
    void createBranch(const std::string& name, const Hash& commitHash);
    void deleteBranch(const std::string& name);
    std::vector<std::string> listBranches();
    std::string getCurrentBranch();
    
    void createTag(const std::string& name, const Hash& commitHash, const std::string& message = "");
    void deleteTag(const std::string& name);
    std::vector<std::string> listTags();
    
    Hash commit(const std::string& message);
    std::vector<Hash> getCommitHistory(const Hash& startCommit);
    
    void checkout(const Hash& commitHash);
    void checkoutBranch(const std::string& branchName);
    
    void add(const std::string& path);
    void remove(const std::string& path);
    std::unordered_map<std::string, Hash> getIndexEntries();

private:
    fs::path repoPath;
    fs::path motorDir;
    
    Hash hashData(const std::string& data);
    fs::path getObjectPath(const Hash& hash);
    std::string compressData(const std::string& data);
    std::string decompressData(const std::string& data);
    Hash writeTree();
    void readTreeToWorkdir(const Hash& treeHash, const fs::path& path);
};

} // namespace motor

#endif // MOTOR_REPOSITORY_H 