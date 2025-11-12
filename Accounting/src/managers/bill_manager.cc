#include "managers/bill_manager.h"
#include "managers/category_manager.h"
#include "models/user.h"
#include <algorithm>
#include <iostream>

namespace accounting {

// ========================== 添加账单 ==========================
bool BillManager::AddBill(int user_id, Bill bill) {
    auto& bills = bills_[user_id];

    // 初始化 ID 生成器
    if (next_bill_id_.find(user_id) == next_bill_id_.end()) {
        next_bill_id_[user_id] = 1;
    }

    // 自动或指定 ID
    if (bill.GetBillId() == 0) {
        bill.SetBillId(next_bill_id_[user_id]++);
    } else {
        for (const auto& b : bills) {
            if (b.GetBillId() == bill.GetBillId()) {
                std::cerr << "[BillManager] Duplicate bill_id for user "
                          << user_id << ": " << bill.GetBillId() << std::endl;
                return false;
            }
        }
        next_bill_id_[user_id] = std::max(next_bill_id_[user_id], bill.GetBillId() + 1);
    }

    bills.push_back(std::move(bill));
    return true;
}

// ========================== 更新账单 ==========================
bool BillManager::UpdateBill(int user_id, const Bill& updated_bill) {
    auto it = bills_.find(user_id);
    if (it == bills_.end()) return false;

    for (auto& bill : it->second) {
        if (bill.GetBillId() == updated_bill.GetBillId()) {
            bill = updated_bill;
            return true;
        }
    }
    return false;
}

// ========================== 删除账单 ==========================
bool BillManager::DeleteBill(int user_id, int bill_id) {
    auto it = bills_.find(user_id);
    if (it == bills_.end()) return false;

    auto& vec = it->second;
    auto old_size = vec.size();
    vec.erase(std::remove_if(vec.begin(), vec.end(),
                             [bill_id](const Bill& b) {
                                 return b.GetBillId() == bill_id;
                             }),
              vec.end());
    return vec.size() != old_size;
}

// ========================== 获取账单 ==========================
std::vector<Bill> BillManager::GetBillsByUser(int user_id) const {
    auto it = bills_.find(user_id);
    if (it != bills_.end()) return it->second;
    return {};
}

// ========================== 按条件查询 ==========================
std::vector<Bill> BillManager::QueryBillsByCriteria(
    int user_id, const QueryCriteria& criteria) const {
    std::vector<Bill> results;
    auto it = bills_.find(user_id);
    if (it == bills_.end()) return results;

    for (const auto& bill : it->second) {
        bool match = true;

        // 日期范围过滤
        if (criteria.HasDateRange()) {
            if (bill.GetTime() < criteria.GetStartDate() ||
                bill.GetTime() > criteria.GetEndDate()) {
                match = false;
            }
        }

        // 分类名称过滤
        if (criteria.HasCategoryFilter()) {
            auto category = bill.GetCategory();
            if (!category || category->GetName() != criteria.GetCategoryName()) {
                match = false;
            }
        }

        if (match) results.push_back(bill);
    }

    return results;
}

// ========================== 存储加载 ==========================
bool BillManager::LoadFromStorage(std::shared_ptr<Storage> storage, const CategoryManager& category_manager) {
    if (!storage) return false;
    try {
        auto res = storage->LoadBillsByUser();
        if (!res.first) return false;
        bills_ = std::move(res.second);
    } catch (...) {
        return false;
    }
    next_bill_id_.clear();

    for (auto& [user_id, bills] : bills_) {
        int max_id = 0;
        // restore category pointers for each bill
        User tmp_user;
        tmp_user.SetUserId(user_id);
        for (auto& bill : bills) {
            max_id = std::max(max_id, bill.GetBillId());
            int cid = bill.GetCategoryId();
            if (cid >= 0) {
                const Category* c = category_manager.GetCategoryById(tmp_user, cid);
                if (c) {
                    // create shared_ptr copy from manager's Category
                    bill.SetCategory(std::make_shared<Category>(*c));
                } else {
                    bill.SetCategory(nullptr);
                }
            } else {
                bill.SetCategory(nullptr);
            }
        }
        next_bill_id_[user_id] = max_id + 1;
    }
    return true;
}

bool BillManager::SaveToStorage(std::shared_ptr<Storage> storage) const {
    return storage->SaveBillsByUser(bills_);
}

}  // namespace accounting
