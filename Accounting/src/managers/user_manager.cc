#include "managers/user_manager.h"
#include <algorithm>

namespace accounting {

bool UserManager::RegisterUser(const std::string& username, const std::string& password) {
    if (users_.find(username) != users_.end()) {
        return false;  // 用户已存在
    }
    int new_id = GenerateNextUserId();
    User user(new_id, username);
    user.SetPassword(password);
    users_[username] = user;
    return true;
}

std::shared_ptr<User> UserManager::Login(const std::string& username, const std::string& password) const {
    auto it_user = users_.find(username);
    if (it_user != users_.end()) {
        if (it_user->second.GetPassword() == password) {
            return std::make_shared<User>(it_user->second);
        }
    }
    return nullptr;
}

std::map<std::string, std::string> UserManager::LoadPreferences(int user_id) const {
    for (const auto& [username, user] : users_) {
        if (user.GetUserId() == user_id) {
            return user.GetPreferences();
        }
    }
    return {};
}

bool UserManager::SavePreferences(int user_id, const std::map<std::string, std::string>& preferences) {
    for (auto& [username, user] : users_) {
        if (user.GetUserId() == user_id) {
            for (const auto& [key, value] : preferences) {
                user.SetPreference(key, value);
            }
            return true;
        }
    }
    return false;
}

bool UserManager::LoadFromStorage(std::shared_ptr<Storage> storage) {
    if (!storage) return false;
    try {
        auto res = storage->LoadUsers();
        if (!res.first) return false;
        auto loaded_users = std::move(res.second);
        users_.clear();
        for (const auto& user : loaded_users) {
            users_[user.GetUsername()] = user;
        }
    } catch (...) {
        return false;
    }
    return true;
}

bool UserManager::SaveToStorage(std::shared_ptr<Storage> storage) const {
    if (!storage) return false;
    std::vector<User> user_list;
    for (const auto& [username, user] : users_) {
        user_list.push_back(user);
    }
    return storage->SaveUsers(user_list);
}

int UserManager::GenerateNextUserId() const {
    int max_id = 0;
    for (const auto& [_, user] : users_) {
        max_id = std::max(max_id, user.GetUserId());
    }
    return max_id + 1;
}

}  // namespace accounting
