#ifndef ACCOUNTING_MODELS_USER_H_
#define ACCOUNTING_MODELS_USER_H_

#include <map>
#include <string>
#include <nlohmann/json.hpp>

namespace accounting {

using nlohmann::json;

class User {
public:
    // 默认构造
    User();

    // 带参构造
    User(int user_id, const std::string& username);

    // 拷贝与赋值默认即可
    User(const User&) = default;
    User& operator=(const User&) = default;

    // Getter / Setter
    int GetUserId() const;
    void SetUserId(int id);

    const std::string& GetUsername() const;
    void SetUsername(const std::string& username);

    const std::map<std::string, std::string>& GetPreferences() const;
    void SetPreference(const std::string& key, const std::string& value);
    std::string GetPreference(const std::string& key, const std::string& default_value = "") const;

    // 密码（简单示例：明文，建议在生产中使用哈希）
    const std::string& GetPassword() const;
    void SetPassword(const std::string& password);

    // 调试输出
    std::string ToString() const;

private:
    int user_id_;
    std::string username_;
    std::map<std::string, std::string> preferences_;
    std::string password_;

    friend void to_json(json& j, const User& u);
    friend void from_json(const json& j, User& u);

};

}  // namespace accounting

#endif  // ACCOUNTING_MODELS_USER_H_
