#include "core/account_manager.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>

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
bool AccountManager::CheckBudgetBeforeAdd(int user_id, const Bill& bill) const {
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

bool AccountManager::CanAddBill(int user_id, const Bill& bill) const {
    return CheckBudgetBeforeAdd(user_id, bill);
}

// ========== 第一阶段：带错误处理的用户操作 ==========

OperationResult<std::shared_ptr<User>> AccountManager::RegisterUserEx(
    const std::string& username, const std::string& password) {
    // 验证输入
    auto validation = ValidateUserInput(username, password);
    if (!validation.IsSuccess()) {
        return OperationResult<std::shared_ptr<User>>::Failure(
            validation.GetErrorCode(),
            validation.GetErrorMessage()
        );
    }

    // 尝试注册
    if (!user_manager_.RegisterUser(username, password)) {
        return OperationResult<std::shared_ptr<User>>::Failure(
            ErrorCode::UserAlreadyExists,
            "用户名已存在，请使用其他用户名"
        );
    }

    // 注册成功，尝试登录
    auto user = user_manager_.Login(username, password);
    if (!user) {
        return OperationResult<std::shared_ptr<User>>::Failure(
            ErrorCode::UnknownError,
            "注册后登录失败，请重试"
        );
    }

    return OperationResult<std::shared_ptr<User>>::Success(user);
}

OperationResult<std::shared_ptr<User>> AccountManager::LoginEx(
    const std::string& username, const std::string& password) {
    // 验证输入
    auto validation = ValidateUserInput(username, password);
    if (!validation.IsSuccess()) {
        return OperationResult<std::shared_ptr<User>>::Failure(
            validation.GetErrorCode(),
            validation.GetErrorMessage()
        );
    }

    // 尝试登录
    auto user = user_manager_.Login(username, password);
    if (!user) {
        return OperationResult<std::shared_ptr<User>>::Failure(
            ErrorCode::PasswordMismatch,
            "用户名或密码错误"
        );
    }

    return OperationResult<std::shared_ptr<User>>::Success(user);
}

// ========== 第一阶段：带错误处理的账单操作 ==========

OperationResult<void> AccountManager::AddBillEx(int user_id, const Bill& bill) {
    // 验证账单
    auto validation = ValidateBill(bill);
    if (!validation.IsSuccess()) {
        return OperationResult<void>::Failure(
            validation.GetErrorCode(),
            validation.GetErrorMessage()
        );
    }

    // 检查预算
    if (!CheckBudgetBeforeAdd(user_id, bill)) {
        return OperationResult<void>::Failure(
            ErrorCode::BudgetExceeded,
            "添加该账单将超过预算限制"
        );
    }

    // 尝试添加
    if (!bill_manager_.AddBill(user_id, bill)) {
        return OperationResult<void>::Failure(
            ErrorCode::StorageError,
            "账单添加失败，请重试"
        );
    }

    report_manager_->ClearReports(user_id);
    return OperationResult<void>::Success();
}

OperationResult<void> AccountManager::UpdateBillEx(int user_id, const Bill& bill) {
    // 验证账单
    auto validation = ValidateBill(bill);
    if (!validation.IsSuccess()) {
        return OperationResult<void>::Failure(
            validation.GetErrorCode(),
            validation.GetErrorMessage()
        );
    }

    // 尝试更新
    if (!bill_manager_.UpdateBill(user_id, bill)) {
        return OperationResult<void>::Failure(
            ErrorCode::BillNotFound,
            "账单不存在或更新失败"
        );
    }

    report_manager_->ClearReports(user_id);
    return OperationResult<void>::Success();
}

OperationResult<void> AccountManager::DeleteBillEx(int user_id, int bill_id) {
    if (!bill_manager_.DeleteBill(user_id, bill_id)) {
        return OperationResult<void>::Failure(
            ErrorCode::BillNotFound,
            "账单不存在或删除失败"
        );
    }

    report_manager_->ClearReports(user_id);
    return OperationResult<void>::Success();
}

// ========== 第一阶段：带错误处理的分类操作 ==========

OperationResult<void> AccountManager::AddCategoryEx(const User& user,
                                                    const Category& category) {
    // 验证分类
    auto validation = ValidateCategory(user, category);
    if (!validation.IsSuccess()) {
        return OperationResult<void>::Failure(
            validation.GetErrorCode(),
            validation.GetErrorMessage()
        );
    }

    // 尝试添加
    if (!category_manager_.AddCategory(user, category)) {
        return OperationResult<void>::Failure(
            ErrorCode::StorageError,
            "分类添加失败，可能已存在相同名称"
        );
    }

    return OperationResult<void>::Success();
}

OperationResult<void> AccountManager::UpdateCategoryEx(const User& user,
                                                       const Category& category) {
    // 验证分类
    auto validation = ValidateCategory(user, category);
    if (!validation.IsSuccess()) {
        return OperationResult<void>::Failure(
            validation.GetErrorCode(),
            validation.GetErrorMessage()
        );
    }

    // 尝试更新
    if (!category_manager_.UpdateCategory(user, category)) {
        return OperationResult<void>::Failure(
            ErrorCode::CategoryNotFound,
            "分类不存在或更新失败"
        );
    }

    return OperationResult<void>::Success();
}

OperationResult<void> AccountManager::DeleteCategoryEx(const User& user,
                                                       int category_id) {
    if (!category_manager_.DeleteCategory(user, category_id)) {
        return OperationResult<void>::Failure(
            ErrorCode::CategoryNotFound,
            "分类不存在或删除失败"
        );
    }

    return OperationResult<void>::Success();
}

// ========== 第一阶段：带错误处理的预算操作 ==========

OperationResult<void> AccountManager::SetBudgetEx(int user_id, const Budget& budget) {
    // 验证预算
    auto validation = ValidateBudget(budget);
    if (!validation.IsSuccess()) {
        return OperationResult<void>::Failure(
            validation.GetErrorCode(),
            validation.GetErrorMessage()
        );
    }

    // 尝试设置
    if (!budget_manager_.SetBudget(user_id, budget)) {
        return OperationResult<void>::Failure(
            ErrorCode::StorageError,
            "预算设置失败，请重试"
        );
    }

    return OperationResult<void>::Success();
}

// ========== 第二阶段：数据验证接口 ==========

OperationResult<void> AccountManager::ValidateUserInput(const std::string& username,
                                                       const std::string& password) const {
    // 验证用户名
    if (username.empty()) {
        return OperationResult<void>::Failure(
            ErrorCode::InvalidUsername,
            "用户名不能为空"
        );
    }

    if (username.length() < 3) {
        return OperationResult<void>::Failure(
            ErrorCode::InvalidUsername,
            "用户名长度至少为 3 个字符"
        );
    }

    if (username.length() > 32) {
        return OperationResult<void>::Failure(
            ErrorCode::InvalidUsername,
            "用户名长度不能超过 32 个字符"
        );
    }

    // 验证密码
    if (password.empty()) {
        return OperationResult<void>::Failure(
            ErrorCode::InvalidPassword,
            "密码不能为空"
        );
    }

    if (password.length() < 6) {
        return OperationResult<void>::Failure(
            ErrorCode::InvalidPassword,
            "密码长度至少为 6 个字符"
        );
    }

    if (password.length() > 64) {
        return OperationResult<void>::Failure(
            ErrorCode::InvalidPassword,
            "密码长度不能超过 64 个字符"
        );
    }

    return OperationResult<void>::Success();
}

OperationResult<void> AccountManager::ValidateBill(const Bill& bill) const {
    // 验证金额
    if (bill.GetAmount() <= 0) {
        return OperationResult<void>::Failure(
            ErrorCode::InvalidBill,
            "账单金额必须大于 0"
        );
    }

    if (bill.GetAmount() > 1000000) {  // 上限为 100 万
        return OperationResult<void>::Failure(
            ErrorCode::InvalidBill,
            "账单金额不能超过 1000000"
        );
    }

    // 验证时间（使用 time_point，不依赖字符串格式）
    // 这里简单检查时间是否合理：不早于 1970-01-01，且不晚于当前时间 + 1 天
    auto tp = bill.GetTime();
    auto now = std::chrono::system_clock::now();
    auto earliest = std::chrono::system_clock::from_time_t(0);
    if (tp < earliest || tp > now + std::chrono::hours(24)) {
        return OperationResult<void>::Failure(
            ErrorCode::InvalidBill,
            "账单时间不合理"
        );
    }

    // 验证描述（content）
    if (bill.GetContent().length() > 256) {
        return OperationResult<void>::Failure(
            ErrorCode::InvalidBill,
            "描述长度不能超过 256 个字符"
        );
    }

    return OperationResult<void>::Success();
}

OperationResult<void> AccountManager::ValidateCategory(const User& user,
                                                       const Category& category) const {
    // 验证分类名称
    if (category.GetName().empty()) {
        return OperationResult<void>::Failure(
            ErrorCode::InvalidCategory,
            "分类名称不能为空"
        );
    }

    if (category.GetName().length() > 64) {
        return OperationResult<void>::Failure(
            ErrorCode::InvalidCategory,
            "分类名称长度不能超过 64 个字符"
        );
    }

    return OperationResult<void>::Success();
}

OperationResult<void> AccountManager::ValidateBudget(const Budget& budget) const {
    // 验证总预算
    if (budget.GetTotalLimit() <= 0) {
        return OperationResult<void>::Failure(
            ErrorCode::InvalidBudget,
            "总预算必须大于 0"
        );
    }

    if (budget.GetTotalLimit() > 100000000) {  // 上限为 1 亿
        return OperationResult<void>::Failure(
            ErrorCode::InvalidBudget,
            "总预算不能超过 100000000"
        );
    }

    // 验证分类预算限额
    const auto& limits = budget.GetCategoryLimits();
    for (const auto& [category_id, limit] : limits) {
        if (limit <= 0) {
            return OperationResult<void>::Failure(
                ErrorCode::InvalidBudget,
                "分类预算限额必须大于 0"
            );
        }

        if (limit > budget.GetTotalLimit()) {
            return OperationResult<void>::Failure(
                ErrorCode::InvalidBudget,
                "分类预算限额不能超过总预算"
            );
        }
    }

    return OperationResult<void>::Success();
}

// ========== 第三阶段：高效查询接口 ==========

std::vector<Bill> AccountManager::GetBillsByDateRange(
    int user_id, const std::string& start_date, const std::string& end_date) const {
    // 验证日期格式
    if (!IsValidDateFormat(start_date) || !IsValidDateFormat(end_date)) {
        return {};
    }

    // 验证日期范围
    if (!IsDateLessOrEqual(start_date, end_date)) {
        return {};
    }

    // 解析起止日期为 time_point
    std::chrono::system_clock::time_point tp_start, tp_end;
    if (!ParseDateStringToTimePoint(start_date, tp_start) ||
        !ParseDateStringToTimePoint(end_date, tp_end)) {
        return {};
    }

    // 获取所有账单
    auto all_bills = GetBills(user_id);

    // 按日期范围过滤（比较 time_point）
    std::vector<Bill> result;
    for (const auto& bill : all_bills) {
        auto bill_tp = bill.GetTime();
        if (bill_tp >= tp_start && bill_tp <= tp_end) {
            result.push_back(bill);
        }
    }

    return result;
}

std::vector<Bill> AccountManager::GetBillsByCategory(int user_id, int category_id) const {
    auto all_bills = GetBills(user_id);
    
    std::vector<Bill> result;
    for (const auto& bill : all_bills) {
        if (bill.GetCategoryId() == category_id) {
            result.push_back(bill);
        }
    }

    return result;
}

std::vector<Bill> AccountManager::GetBillsByCategoryAndDate(
    int user_id, int category_id, const std::string& start_date,
    const std::string& end_date) const {
    // 先按分类过滤
    auto bills_by_category = GetBillsByCategory(user_id, category_id);

    // 再按日期范围过滤
    if (!IsValidDateFormat(start_date) || !IsValidDateFormat(end_date)) {
        return {};
    }

    if (!IsDateLessOrEqual(start_date, end_date)) {
        return {};
    }

    std::vector<Bill> result;
    // 解析起止日期为 time_point
    std::chrono::system_clock::time_point tp_start, tp_end;
    if (!ParseDateStringToTimePoint(start_date, tp_start) ||
        !ParseDateStringToTimePoint(end_date, tp_end)) {
        return {};
    }

    for (const auto& bill : bills_by_category) {
        auto bill_tp = bill.GetTime();
        if (bill_tp >= tp_start && bill_tp <= tp_end) {
            result.push_back(bill);
        }
    }

    return result;
}

PagedResult<Bill> AccountManager::GetBillsPaged(int user_id, int page_number,
                                               int page_size) const {
    PagedResult<Bill> result;
    result.page_number = page_number;
    result.page_size = page_size;

    auto all_bills = GetBills(user_id);
    result.total_count = all_bills.size();
    result.CalculateTotalPages();

    // 验证页码
    if (page_number < 1 || (page_number > result.total_pages && result.total_pages > 0)) {
        return result;  // 返回空结果
    }

    // 计算起始和结束索引
    int start_idx = (page_number - 1) * page_size;
    int end_idx = std::min(start_idx + page_size, (int)all_bills.size());

    result.items.assign(all_bills.begin() + start_idx, all_bills.begin() + end_idx);

    return result;
}

double AccountManager::GetTotalExpenseByCategory(
    int user_id, int category_id, const std::string& start_date,
    const std::string& end_date) const {
    auto bills = GetBillsByCategoryAndDate(user_id, category_id, start_date, end_date);

    double total = 0.0;
    for (const auto& bill : bills) {
        total += bill.GetAmount();
    }

    return total;
}

double AccountManager::GetTotalExpense(int user_id, const std::string& start_date,
                                      const std::string& end_date) const {
    auto bills = GetBillsByDateRange(user_id, start_date, end_date);

    double total = 0.0;
    for (const auto& bill : bills) {
        total += bill.GetAmount();
    }

    return total;
}

// ========== 第四阶段：预算分析接口 ==========

BudgetStatus AccountManager::GetBudgetStatus(int user_id) const {
    BudgetStatus status;

    auto budget = GetBudget(user_id);
    if (!budget) {
        status.budget_set = false;
        return status;
    }

    status.budget_set = true;
    status.total_budget = budget->GetTotalLimit();

    // 计算已使用金额（当月？还是全部？为简化起见，这里使用全部）
    auto bills = GetBills(user_id);
    for (const auto& bill : bills) {
        status.used_amount += bill.GetAmount();
    }

    status.remaining_budget = status.total_budget - status.used_amount;
    status.is_exceeded = status.remaining_budget < 0;

    if (status.total_budget > 0) {
        status.usage_percentage = status.used_amount / status.total_budget;
    }

    return status;
}

std::vector<CategoryBudgetStatus> AccountManager::GetCategoryBudgetStatus(
    int user_id) const {
    std::vector<CategoryBudgetStatus> result;

    auto budget = GetBudget(user_id);
    if (!budget) {
        return result;  // 未设置预算，返回空
    }

    const auto& limits = budget->GetCategoryLimits();

    // 遍历预算中的所有分类
    for (const auto& [category_id, limit] : limits) {
        CategoryBudgetStatus cat_status;
        cat_status.category_id = category_id;
        cat_status.category_name = "分类 #" + std::to_string(category_id);  // 默认名称
        cat_status.limit = limit;
        cat_status.limit_set = true;

        // 计算该分类的使用金额
        auto bills_in_category = GetBillsByCategory(user_id, category_id);
        for (const auto& bill : bills_in_category) {
            cat_status.used += bill.GetAmount();
        }

        cat_status.remaining = limit - cat_status.used;
        cat_status.is_exceeded = cat_status.remaining < 0;

        if (limit > 0) {
            cat_status.usage_percentage = cat_status.used / limit;
        }

        result.push_back(cat_status);
    }

    return result;
}

BudgetImpact AccountManager::GetBudgetImpactIfAddBill(int user_id,
                                                     const Bill& bill) const {
    BudgetImpact impact;

    auto budget = GetBudget(user_id);
    if (!budget) {
        // 未设置预算，不会有影响
        impact.would_exceed_total = false;
        impact.would_exceed_category = false;
        return impact;
    }

    // 获取当前预算状态
    auto current_status = GetBudgetStatus(user_id);
    impact.current_remaining_total = current_status.remaining_budget;
    impact.remaining_total_after_add = current_status.remaining_budget - bill.GetAmount();
    impact.would_exceed_total = impact.remaining_total_after_add < 0;

    // 获取分类预算状态
    const auto& limits = budget->GetCategoryLimits();
    auto it = limits.find(bill.GetCategoryId());
    if (it != limits.end()) {
        double category_limit = it->second;
        auto bills_in_category = GetBillsByCategory(user_id, bill.GetCategoryId());

        double used = 0.0;
        for (const auto& b : bills_in_category) {
            used += b.GetAmount();
        }

        impact.current_remaining_category = category_limit - used;
        impact.remaining_category_after_add = category_limit - (used + bill.GetAmount());
        impact.would_exceed_category = impact.remaining_category_after_add < 0;
    }

    // 生成警告消息
    if (impact.would_exceed_total && impact.would_exceed_category) {
        impact.warning_message =
            "警告：该账单将同时超过总预算和分类预算！";
    } else if (impact.would_exceed_total) {
        impact.warning_message =
            "警告：该账单将超过总预算。剩余预算将变为：" +
            std::to_string(impact.remaining_total_after_add);
    } else if (impact.would_exceed_category) {
        impact.warning_message =
            "警告：该账单将超过该分类的预算。剩余预算将变为：" +
            std::to_string(impact.remaining_category_after_add);
    }

    return impact;
}

// ========== 内部辅助方法 ==========

bool AccountManager::IsValidDateFormat(const std::string& date_str) const {
    // 简单的日期格式验证：YYYY-MM-DD
    if (date_str.length() != 10) {
        return false;
    }

    if (date_str[4] != '-' || date_str[7] != '-') {
        return false;
    }

    // 检查年、月、日都是数字
    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(date_str[i])) {
            return false;
        }
    }

    // 进一步验证月和日的范围
    int month = std::stoi(date_str.substr(5, 2));
    int day = std::stoi(date_str.substr(8, 2));

    if (month < 1 || month > 12 || day < 1 || day > 31) {
        return false;
    }

    return true;
}

bool AccountManager::IsDateLessOrEqual(const std::string& date1,
                                      const std::string& date2) const {
    // 简单的字符串比较（YYYY-MM-DD 格式可直接比较）
    return date1 <= date2;
}

bool AccountManager::ParseDateStringToTimePoint(const std::string& date_str,
                                               std::chrono::system_clock::time_point& out) const {
    if (!IsValidDateFormat(date_str)) return false;

    int year = std::stoi(date_str.substr(0, 4));
    int month = std::stoi(date_str.substr(5, 2));
    int day = std::stoi(date_str.substr(8, 2));

    std::tm tm{};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;

    std::time_t tt = std::mktime(&tm);
    if (tt == -1) return false;

    out = std::chrono::system_clock::from_time_t(tt);
    return true;
}

bool AccountManager::ParseDateTimeStringToTimePoint(const std::string& date_str,
                                                    const std::string& time_str,
                                                    std::chrono::system_clock::time_point& out) const {
    if (!IsValidDateFormat(date_str)) return false;

    int year = std::stoi(date_str.substr(0, 4));
    int month = std::stoi(date_str.substr(5, 2));
    int day = std::stoi(date_str.substr(8, 2));

    int hour = 0, min = 0, sec = 0;
    // parse time_str like HH:MM or HH:MM:SS
    std::sscanf(time_str.c_str(), "%d:%d:%d", &hour, &min, &sec);

    std::tm tm{};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;

    std::time_t tt = std::mktime(&tm);
    if (tt == -1) return false;

    out = std::chrono::system_clock::from_time_t(tt);
    return true;
}

std::pair<double, double> AccountManager::GetDailySummary(int user_id, const std::string& date_str) const {
    std::pair<double,double> res{0.0, 0.0};
    std::chrono::system_clock::time_point tp_start;
    if (!ParseDateStringToTimePoint(date_str, tp_start)) return res;
    auto tp_end = tp_start + std::chrono::hours(24);

    auto bills = GetBills(user_id);
    for (const auto& bill : bills) {
        auto t = bill.GetTime();
        if (t >= tp_start && t < tp_end) {
            auto cat = bill.GetCategory();
            if (cat && cat->GetType() == "income") {
                res.first += bill.GetAmount();
            } else {
                res.second += bill.GetAmount();
            }
        }
    }

    return res;
}

}  // namespace accounting
