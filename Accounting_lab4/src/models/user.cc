#include "models/user.h"
#include <sstream>

namespace accounting {

// 默认构造
User::User() : user_id_(0), username_("") {}

// 带参构造
User::User(int user_id, const std::string& username)
    : user_id_(user_id), username_(username) {}

const std::string& User::GetPassword() const { return password_; }
void User::SetPassword(const std::string& password) { password_ = password; }

// Getter / Setter
int User::GetUserId() const { return user_id_; }
void User::SetUserId(int id) { user_id_ = id; }

const std::string& User::GetUsername() const { return username_; }
void User::SetUsername(const std::string& username) { username_ = username; }

const std::map<std::string, std::string>& User::GetPreferences() const { return preferences_; }

void User::SetPreference(const std::string& key, const std::string& value) {
  preferences_[key] = value;
}

std::string User::GetPreference(const std::string& key, const std::string& default_value) const {
    auto it = preferences_.find(key);
    if (it != preferences_.end()) {
        return it->second;
    }
    return default_value;
}

// 调试输出
std::string User::ToString() const {
    std::ostringstream oss;
    oss << "User(ID: " << user_id_ << ", Username: " << username_ << ", Preferences: {";
    bool first = true;
    for (const auto& [key, value] : preferences_) {
        if (!first) oss << ", ";
        oss << key << ": " << value;
        first = false;
    }
    oss << "})";
    return oss.str();
}

// ==== JSON Support ====
void to_json(json& j, const User& u) {
    j = json{
        {"user_id", u.user_id_},
        {"username", u.username_},
        {"preferences", u.preferences_},
        {"password", u.password_}
    };
}

void from_json(const json& j, User& u) {
    u.user_id_ = j.at("user_id").get<int>();
    u.username_ = j.at("username").get<std::string>();
    if (j.contains("preferences"))
        u.preferences_ = j.at("preferences").get<std::map<std::string, std::string>>();
    else
        u.preferences_.clear();
    if (j.contains("password"))
        u.password_ = j.at("password").get<std::string>();
    else
        u.password_.clear();
}

}  // namespace accounting
