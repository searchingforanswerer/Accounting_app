#include "core/account_manager.h"
#include <iostream>

namespace accounting {

AccountManager::AccountManager(std::shared_ptr<Storage> storage)
    : storage_(std::move(storage)),
      category_manager_(storage_) {
    // ReportManager 依赖 BillManager，因此用 unique_ptr 动态初始化
    report_manager_ = std::make_unique<ReportManager>(&bill_manager_);
}

bool AccountManager::Initialize() {
    // 加载所有数据（文件不存在时视为首次运行且不视为失败；文件存在但解析/IO 错误 -> 初始化失败）
    if (!user_manager_.LoadFromStorage(storage_)) return false;
    if (!category_manager_.LoadFromStorage()) return false;
    if (!budget_manager_.LoadFromStorage(storage_)) return false;
    // BillManager 需要 CategoryManager 已加载，所以放在最后
    if (!bill_manager_.LoadFromStorage(storage_, category_manager_)) return false;
    return true;
}

bool AccountManager::SaveAll() const {
    bool ok = true;
    ok &= user_manager_.SaveToStorage(storage_);
    ok &= bill_manager_.SaveToStorage(storage_);
    ok &= category_manager_.SaveToStorage();
    ok &= budget_manager_.SaveToStorage(storage_);
    return ok;
}

// === 用户 ===
bool AccountManager::RegisterUser(const std::string& username, const std::string& password) {
    return user_manager_.RegisterUser(username, password);
}

std::shared_ptr<User> AccountManager::Login(const std::string& username, const std::string& password) {
    return user_manager_.Login(username, password);
}

// === 账单 ===
bool AccountManager::CheckBudgetBeforeAdd(int user_id, const Bill& bill) {
    auto budget = budget_manager_.GetBudget(user_id);
    if (!budget) return true;  // 没设置预算则直接通过
    return budget_manager_.CheckLimit(user_id, bill);
}

bool AccountManager::AddBill(int user_id, Bill bill) {
    if (!CheckBudgetBeforeAdd(user_id, bill)) {
        std::cerr << "[警告] 账单超出预算限制，未添加。\n";
        return false;
    }
    if (!bill_manager_.AddBill(user_id, bill)) return false;
    report_manager_->ClearReports(user_id);
    return true;
}

bool AccountManager::UpdateBill(int user_id, const Bill& bill) {
    return bill_manager_.UpdateBill(user_id, bill);
}

bool AccountManager::DeleteBill(int user_id, int bill_id) {
    return bill_manager_.DeleteBill(user_id, bill_id);
}

std::vector<Bill> AccountManager::GetBills(int user_id) const {
    return bill_manager_.GetBillsByUser(user_id);
}

std::vector<Bill> AccountManager::QueryBills(int user_id, const QueryCriteria& criteria) const {
    return bill_manager_.QueryBillsByCriteria(user_id, criteria);
}

// === 分类 ===
bool AccountManager::AddCategory(const User& user, const Category& category) {
    return category_manager_.AddCategory(user, category);
}

bool AccountManager::UpdateCategory(const User& user, const Category& category) {
    return category_manager_.UpdateCategory(user, category);
}

bool AccountManager::DeleteCategory(const User& user, int category_id) {
    return category_manager_.DeleteCategory(user, category_id);
}

std::vector<Category> AccountManager::GetCategories(const User& user) const {
    return category_manager_.GetCategoriesForUser(user);
}

// === 预算 ===
bool AccountManager::SetBudget(int user_id, const Budget& budget) {
    return budget_manager_.SetBudget(user_id, budget);
}

std::shared_ptr<Budget> AccountManager::GetBudget(int user_id) const {
    return budget_manager_.GetBudget(user_id);
}

// === 报表 ===
Report AccountManager::GenerateReport(int user_id, const QueryCriteria& criteria,
                                      Period period, ChartType chart_type) {
    return report_manager_->GenerateReport(user_id, criteria, period, chart_type);
}

std::optional<Report> AccountManager::GetLastReport(int user_id) const {
    return report_manager_->GetLastReport(user_id);
}

}  // namespace accounting
