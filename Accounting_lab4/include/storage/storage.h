#ifndef ACCOUNTING_STORAGE_STORAGE_H_
#define ACCOUNTING_STORAGE_STORAGE_H_

#include <map>
#include <vector>
#include <memory>
#include <string>
#include <utility>
#include "models/category.h"
#include "models/bill.h"
#include "models/user.h"
#include "models/budget.h"
#include "models/report.h"

namespace accounting {

// Storage 接口，提供系统数据的读写抽象
class Storage {
public:
    virtual ~Storage() = default;

    // ===== 用户 =====
    // 返回 pair<success, data>. success == false 表示读取/解析发生错误.
    virtual std::pair<bool, std::vector<User>> LoadUsers() = 0;
    virtual bool SaveUsers(const std::vector<User>& users) = 0;

    // ===== 分类 =====
    virtual std::pair<bool, std::map<int, std::vector<Category>>> LoadCategoriesByUser() = 0;
    virtual bool SaveCategoriesByUser(
        const std::map<int, std::vector<Category>>& data) = 0;

    // ===== 账单 =====
    virtual std::pair<bool, std::map<int, std::vector<Bill>>> LoadBillsByUser() = 0;
    virtual bool SaveBillsByUser(
        const std::map<int, std::vector<Bill>>& data) = 0;

    // ===== 预算 =====
    virtual std::pair<bool, std::map<int, Budget>> LoadBudgetsByUser() = 0;
    virtual bool SaveBudgetsByUser(
        const std::map<int, Budget>& data) = 0;

    // NOTE: Report persistence APIs removed — reports are derived data and
    // can be regenerated on demand. If snapshotting is later required,
    // add explicit APIs for report definitions or snapshots.
};

}  // namespace accounting

#endif  // ACCOUNTING_STORAGE_STORAGE_H_
