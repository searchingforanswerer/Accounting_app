#ifndef ACCOUNTING_MANAGERS_USER_MANAGER_H_
#define ACCOUNTING_MANAGERS_USER_MANAGER_H_

#include <unordered_map>
#include <string>
#include <memory>
#include "models/user.h"
#include "storage/storage.h"

namespace accounting {

class UserManager {
public:
    UserManager() = default;

    // ==== 用户注册/登录 ====
    // 注册新用户，如果用户名已存在则返回 false
    bool RegisterUser(const std::string& username, const std::string& password);

    // 登录，成功返回 User 对象，失败返回空指针
    std::shared_ptr<User> Login(const std::string& username, const std::string& password) const;

    // ==== 用户偏好设置 ====
    // 获取用户偏好
    std::map<std::string, std::string> LoadPreferences(int user_id) const;

    // 保存用户偏好
    bool SavePreferences(int user_id, const std::map<std::string, std::string>& preferences);

    // ==== 存储操作 ====
    bool LoadFromStorage(std::shared_ptr<Storage> storage);
    bool SaveToStorage(std::shared_ptr<Storage> storage) const;

private:
    // 用户名 -> User
    std::unordered_map<std::string, User> users_;

    // 密码现在保存在 User 对象中（User::password_）

    // 为新用户生成唯一 ID
    int GenerateNextUserId() const;
};

}  // namespace accounting

#endif  // ACCOUNTING_MANAGERS_USER_MANAGER_H_
