#ifndef ACCOUNTING_STORAGE_JSON_STORAGE_H_
#define ACCOUNTING_STORAGE_JSON_STORAGE_H_

#include "storage/storage.h"
#include <string>
#include <filesystem>

namespace accounting {

/**
 * @brief 基于 JSON 文件的存储实现类
 * 
 * JsonStorage 实现了 Storage 接口，
 * 使用 JSON 文件在本地进行数据持久化。
 * 每个数据类型对应一个独立的文件。
 */
class JsonStorage : public Storage {
public:
    explicit JsonStorage(const std::string& base_path);
    ~JsonStorage() override = default;

    // 用户
    std::pair<bool, std::vector<User>> LoadUsers() override;
    bool SaveUsers(const std::vector<User>& users) override;

    // 分类
    std::pair<bool, std::map<int, std::vector<Category>>> LoadCategoriesByUser() override;
    bool SaveCategoriesByUser(const std::map<int, std::vector<Category>>& data) override;

    // 账单
    std::pair<bool, std::map<int, std::vector<Bill>>> LoadBillsByUser() override;
    bool SaveBillsByUser(const std::map<int, std::vector<Bill>>& data) override;

    // 预算
    std::pair<bool, std::map<int, Budget>> LoadBudgetsByUser() override;
    bool SaveBudgetsByUser(const std::map<int, Budget>& data) override;

    // Report persistence intentionally omitted (reports are derived).

private:
    std::string base_path_;  // 存放所有 JSON 文件的路径

    // 工具函数：通用的 JSON 读写模板函数
    template<typename T>
    bool SaveToJson(const std::string& filename, const T& data);

    template<typename T>
    bool LoadFromJson(const std::string& filename, T& data);
};

}  // namespace accounting

#endif  // ACCOUNTING_STORAGE_JSON_STORAGE_H_
