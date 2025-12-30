#include "managers/budget_manager.h"
#include <algorithm>

namespace accounting {

bool BudgetManager::SetBudget(int user_id, const Budget& budget) {
    budgets_[user_id] = budget;
    return true;
}

std::shared_ptr<Budget> BudgetManager::GetBudget(int user_id) const {
    auto it = budgets_.find(user_id);
    if (it != budgets_.end()) {
        return std::make_shared<Budget>(it->second);
    }
    return nullptr;
}

bool BudgetManager::CheckLimit(int user_id, const Bill& bill) const {
    auto it = budgets_.find(user_id);
    if (it == budgets_.end()) {
        // 没有设置预算，则默认不限制
        return true;
    }

    const Budget& budget = it->second;
    double total_limit = budget.GetTotalLimit();
    double current_total = 0.0;

    // 计算当前已使用预算（按 category_id -> limit）
    auto category_limits = budget.GetCategoryLimits();
    for (const auto& [category_id, limit] : category_limits) {
        if (bill.GetCategory() && category_id == bill.GetCategory()->GetCategoryId()) {
            // 假设当前账单就是本类别，检查是否超出类别限额
            if (bill.GetAmount() > limit) {
                return false;
            }
        }
        current_total += limit;
    }

    // 检查总预算
    if (bill.GetAmount() > total_limit) {
        return false;
    }

    return true;
}

bool BudgetManager::LoadFromStorage(std::shared_ptr<Storage> storage) {
    if (!storage) return false;
    try {
        auto res = storage->LoadBudgetsByUser();
        if (!res.first) return false;
        budgets_ = std::move(res.second);
    } catch (...) {
        return false;
    }
    return true;
}

bool BudgetManager::SaveToStorage(std::shared_ptr<Storage> storage) const {
    if (!storage) return false;
    return storage->SaveBudgetsByUser(budgets_);
}

}  // namespace accounting
