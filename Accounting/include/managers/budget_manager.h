#ifndef ACCOUNTING_MANAGERS_BUDGET_MANAGER_H_
#define ACCOUNTING_MANAGERS_BUDGET_MANAGER_H_

#include <map>
#include <memory>
#include "models/budget.h"
#include "models/bill.h"
#include "models/user.h"
#include "storage/storage.h"

namespace accounting {

class BudgetManager {
public:
    BudgetManager() = default;

    // 设置用户预算
    bool SetBudget(int user_id, const Budget& budget);

    // 获取用户预算（可选）
    std::shared_ptr<Budget> GetBudget(int user_id) const;

    // 检查账单是否超过预算
    // 返回 true 表示在预算范围内，false 表示超出预算
    bool CheckLimit(int user_id, const Bill& bill) const;

    // 存储操作
    bool LoadFromStorage(std::shared_ptr<Storage> storage);
    bool SaveToStorage(std::shared_ptr<Storage> storage) const;

private:
    // user_id -> Budget (使用 map 与 Storage API 返回类型一致)
    std::map<int, Budget> budgets_;
};

}  // namespace accounting

#endif  // ACCOUNTING_MANAGERS_BUDGET_MANAGER_H_
