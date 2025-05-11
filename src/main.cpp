#include "motor/motor.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <filesystem>
#include <windows.h>

namespace fs = std::filesystem;

void printUsage() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    std::cout << "Используйте: motor <команда> [<аргументы>]\n\n";
    std::cout << "Команды:\n";
    std::cout << "  init                           Создать пустой репозиторий\n";
    std::cout << "  add <файл/каталог>             Добавить файл или каталог в индекс\n";
    std::cout << "  commit -m <сообщение>          Записать изменения в хранилище\n";
    std::cout << "  branch                         Список веток\n";
    std::cout << "  branch <имя>                   Создать новую ветку\n";
    std::cout << "  branch -d <имя>                Удалить ветку\n";
    std::cout << "  checkout <ветка>               Переключиться на ветку\n";
    std::cout << "  checkout <хеш>                 Переключиться на коммит\n";
    std::cout << "  tag                            Список тегов\n";
    std::cout << "  tag <имя>                      Создать легкий тег\n";
    std::cout << "  tag -a <имя> -m <сообщение>    Создать аннотированный тег\n";
    std::cout << "  tag -d <имя>                   Удалить тег\n";
    std::cout << "  log                            Показать журнал коммитов\n";
    std::cout << "  status                         Показать статус рабочего каталога\n";
}

motor::Repository getCurrentRepo() {
    fs::path current = fs::current_path();
    fs::path original = current;
    
    while (true) {
        if (fs::exists(current / ".motor")) {
            return motor::Repository(current.string());
        }
        
        if (current == current.parent_path()) {
            break;
        }
        
        current = current.parent_path();
    }
    
    throw std::runtime_error("Не найден репозиторий Motor (или родительский каталог): " + original.string());
}

void cmdInit(const std::vector<std::string>& args) {
    if (args.size() > 0) {
        motor::Repository::init(args[0]);
    } else {
        motor::Repository::init(".");
    }
    std::cout << "Инициализирован пустой репозиторий Motor\n";
}

void cmdAdd(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Ничего не указано, ничего не добавлено.\n";
        return;
    }
    
    auto repo = getCurrentRepo();
    
    for (const auto& path : args) {
        try {
            repo.add(path);
            std::cout << "Добавлено: " << path << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Ошибка добавления " << path << ": " << e.what() << "\n";
        }
    }
}

void cmdCommit(const std::vector<std::string>& args) {
    if (args.size() < 2 || args[0] != "-m") {
        std::cerr << "Использование: motor commit -m <сообщение>\n";
        return;
    }
    
    auto repo = getCurrentRepo();
    
    try {
        motor::Hash commitHash = repo.commit(args[1]);
        std::cout << "Создан коммит " << commitHash << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Ошибка создания коммита: " << e.what() << "\n";
    }
}

void cmdBranch(const std::vector<std::string>& args) {
    auto repo = getCurrentRepo();
    
    if (args.empty()) {
        std::string currentBranch = repo.getCurrentBranch();
        auto branches = repo.listBranches();
        
        for (const auto& branch : branches) {
            if (branch == currentBranch) {
                std::cout << "* " << branch << " (текущая)\n";
            } else {
                std::cout << "  " << branch << "\n";
            }
        }
    } else if (args[0] == "-d" && args.size() > 1) {
        try {
            repo.deleteBranch(args[1]);
            std::cout << "Удалена ветка " << args[1] << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Ошибка удаления ветки: " << e.what() << "\n";
        }
    } else {
        try {
            std::string branchName = args[0];
            
            std::string currentBranch = repo.getCurrentBranch();
            motor::Hash commitHash;
            
            if (!currentBranch.empty()) {
                commitHash = repo.readRef("refs/heads/" + currentBranch);
            } else {
                fs::path headPath = repo.getMotorDir() / "HEAD";
                std::ifstream file(headPath);
                std::string content;
                std::getline(file, content);
                file.close();
                commitHash = content;
            }
            
            repo.createBranch(branchName, commitHash);
            std::cout << "Создана ветка " << branchName << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Ошибка создания ветки: " << e.what() << "\n";
        }
    }
}

void cmdCheckout(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Использование: motor checkout <имя-ветки> или motor checkout <хеш-коммита>\n";
        return;
    }
    
    auto repo = getCurrentRepo();
    
    try {
        try {
            repo.checkoutBranch(args[0]);
            std::cout << "Переключение на ветку '" << args[0] << "'\n";
        } catch (const std::exception& e) {
            repo.checkout(args[0]);
            std::cout << "HEAD теперь на коммите " << args[0] << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Ошибка переключения: " << e.what() << "\n";
    }
}

void cmdTag(const std::vector<std::string>& args) {
    auto repo = getCurrentRepo();
    
    if (args.empty()) {
        auto tags = repo.listTags();
        for (const auto& tag : tags) {
            std::cout << tag << "\n";
        }
    } else if (args[0] == "-d" && args.size() > 1) {
        try {
            repo.deleteTag(args[1]);
            std::cout << "Удален тег " << args[1] << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Ошибка удаления тега: " << e.what() << "\n";
        }
    } else if (args[0] == "-a" && args.size() >= 4 && args[2] == "-m") {
        try {
            std::string tagName = args[1];
            std::string message = args[3];
            
            std::string currentBranch = repo.getCurrentBranch();
            motor::Hash commitHash;
            
            if (!currentBranch.empty()) {
                commitHash = repo.readRef("refs/heads/" + currentBranch);
            } else {
                fs::path headPath = repo.getMotorDir() / "HEAD";
                std::ifstream file(headPath);
                std::string content;
                std::getline(file, content);
                commitHash = content;
                file.close();
            }
            
            repo.createTag(tagName, commitHash, message);
            std::cout << "Создан тег " << tagName << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Ошибка создания тега: " << e.what() << "\n";
        }
    } else {
        try {
            std::string tagName = args[0];
            
            std::string currentBranch = repo.getCurrentBranch();
            motor::Hash commitHash;
            
            if (!currentBranch.empty()) {
                commitHash = repo.readRef("refs/heads/" + currentBranch);
            } else {
                fs::path headPath = repo.getMotorDir() / "HEAD";
                std::ifstream file(headPath);
                std::string content;
                std::getline(file, content);
                commitHash = content;
                file.close();
            }
            
            repo.createTag(tagName, commitHash);
            std::cout << "Создан тег " << tagName << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Ошибка создания тега: " << e.what() << "\n";
        }
    }
}

void cmdLog(const std::vector<std::string>& args) {
    auto repo = getCurrentRepo();
    
    try {
        std::string currentBranch = repo.getCurrentBranch();
        motor::Hash startCommit;
        
        if (!currentBranch.empty()) {
            startCommit = repo.readRef("refs/heads/" + currentBranch);
        } else {
            fs::path headPath = repo.getMotorDir() / "HEAD";
            std::ifstream file(headPath);
            std::string content;
            std::getline(file, content);
            startCommit = content;
            file.close();
        }
        
        auto history = repo.getCommitHistory(startCommit);
        
        for (const auto& commitHash : history) {
            std::cout << "коммит " << commitHash << "\n";
            
            auto object = repo.readObject(commitHash);
            if (object->getType() == motor::ObjectType::COMMIT) {
                auto commit = static_cast<motor::Commit*>(object.get());
                std::cout << "Сообщение: " << commit->getMessage() << "\n\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Ошибка отображения журнала: " << e.what() << "\n";
    }
}

void cmdStatus(const std::vector<std::string>& args) {
    auto repo = getCurrentRepo();
    
    try {
        std::string currentBranch = repo.getCurrentBranch();
        if (!currentBranch.empty()) {
            std::cout << "На ветке " << currentBranch << "\n\n";
        } else {
            fs::path headPath = repo.getMotorDir() / "HEAD";
            std::ifstream file(headPath);
            std::string commitHash;
            std::getline(file, commitHash);
            file.close();
            
            std::cout << "HEAD отсоединен на коммите " << commitHash << "\n\n";
        }
        
        auto indexEntries = repo.getIndexEntries();
        if (!indexEntries.empty()) {
            std::cout << "Изменения, подготовленные для коммита:\n";
            for (const auto& entry : indexEntries) {
                std::cout << "  " << entry.first << "\n";
            }
        } else {
            std::cout << "Нет изменений, подготовленных для коммита\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Ошибка отображения статуса: " << e.what() << "\n";
    }
}

int main(int argc, char* argv[]) {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    
    if (argc < 2) {
        printUsage();
        return 1;
    }
    
    std::string command = argv[1];
    
    std::vector<std::string> args;
    for (int i = 2; i < argc; i++) {
        args.push_back(argv[i]);
    }
    
    try {
        std::map<std::string, std::function<void(const std::vector<std::string>&)>> commands = {
            {"init", cmdInit},
            {"add", cmdAdd},
            {"commit", cmdCommit},
            {"branch", cmdBranch},
            {"checkout", cmdCheckout},
            {"tag", cmdTag},
            {"log", cmdLog},
            {"status", cmdStatus}
        };
        
        auto it = commands.find(command);
        if (it != commands.end()) {
            it->second(args);
        } else {
            std::cerr << "Неизвестная команда: " << command << "\n";
            printUsage();
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
} 