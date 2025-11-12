#include "storage/json_storage.h"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace accounting {

JsonStorage::JsonStorage(const std::string& base_path)
    : base_path_(base_path) {
    std::filesystem::create_directories(base_path_);
}

// =================== 用户 ===================
std::pair<bool, std::vector<User>> JsonStorage::LoadUsers() {
    std::vector<User> users;
    const std::string path = base_path_ + "/users.json";
    if (!std::filesystem::exists(path)) return {true, users};
    bool ok = LoadFromJson(path, users);
    if (!ok) return {false, {}};
    return {true, users};
}

bool JsonStorage::SaveUsers(const std::vector<User>& users) {
    return SaveToJson(base_path_ + "/users.json", users);
}

// =================== 账单 ===================
std::pair<bool, std::map<int, std::vector<Bill>>> JsonStorage::LoadBillsByUser() {
    std::map<int, std::vector<Bill>> data;
    const std::string path = base_path_ + "/bills.json";
    if (!std::filesystem::exists(path)) return {true, data};
    bool ok = LoadFromJson(path, data);
    if (!ok) return {false, {}};
    return {true, data};
}

bool JsonStorage::SaveBillsByUser(const std::map<int, std::vector<Bill>>& data) {
    return SaveToJson(base_path_ + "/bills.json", data);
}

// =================== 分类 ===================
std::pair<bool, std::map<int, std::vector<Category>>> JsonStorage::LoadCategoriesByUser() {
    std::map<int, std::vector<Category>> data;
    const std::string path = base_path_ + "/categories.json";
    if (!std::filesystem::exists(path)) return {true, data};
    bool ok = LoadFromJson(path, data);
    if (!ok) return {false, {}};
    return {true, data};
}

bool JsonStorage::SaveCategoriesByUser(const std::map<int, std::vector<Category>>& data) {
    return SaveToJson(base_path_ + "/categories.json", data);
}

// =================== 预算 ===================
std::pair<bool, std::map<int, Budget>> JsonStorage::LoadBudgetsByUser() {
    std::map<int, Budget> data;
    const std::string path = base_path_ + "/budgets.json";
    if (!std::filesystem::exists(path)) return {true, data};
    bool ok = LoadFromJson(path, data);
    if (!ok) return {false, {}};
    return {true, data};
}

bool JsonStorage::SaveBudgetsByUser(const std::map<int, Budget>& data) {
    return SaveToJson(base_path_ + "/budgets.json", data);
}

// =================== 通用 JSON 读写 ===================
template<typename T>
bool JsonStorage::SaveToJson(const std::string& filename, const T& data) {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) return false;
        json j = data;
        file << std::setw(4) << j;
        return true;
    } catch (...) {
        return false;
    }
}

template<typename T>
bool JsonStorage::LoadFromJson(const std::string& filename, T& data) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) return false;
        json j;
        file >> j;
        data = j.get<T>();
        return true;
    } catch (...) {
        return false;
    }
}

}  // namespace accounting
