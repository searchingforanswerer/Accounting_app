#ifndef ACCOUNTING_CORE_ACCOUNT_MANAGER_H_
#define ACCOUNTING_CORE_ACCOUNT_MANAGER_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "storage/storage.h"
#include "managers/user_manager.h"
#include "managers/bill_manager.h"
#include "managers/budget_manager.h"
#include "managers/category_manager.h"
#include "managers/report_manager.h"

namespace accounting {

/**
 * @brief AccountManager 是系统的核心调度器。
 *
 * 它负责协调所有 Manager（User/Bill/Budget/Category/Report），
 * 提供统一的接口供上层 UIManager 或 API 调用。
 * 同时封装业务逻辑，如账单添加、预算检查、报表生成等。
 */
class AccountManager {
public:
    /**
     * @brief 构造函数，接受一个共享的 Storage 实例。
     * @param storage 统一的存储层，用于各 Manager 加载和保存数据。
     */
    explicit AccountManager(std::shared_ptr<Storage> storage);

    /**
     * @brief 初始化系统，从存储中加载所有数据。
     * @return 是否加载成功。
     */
    bool Initialize();

    /**
     * @brief 将所有数据保存到存储中。
     * @return 是否保存成功。
     */
    bool SaveAll() const;

    // === 用户相关操作 ===

    bool RegisterUser(const std::string& username, const std::string& password);
    std::shared_ptr<User> Login(const std::string& username, const std::string& password);

    // === 账单相关操作 ===

    bool AddBill(int user_id, Bill bill);
    bool UpdateBill(int user_id, const Bill& bill);
    bool DeleteBill(int user_id, int bill_id);
    std::vector<Bill> GetBills(int user_id) const;
    std::vector<Bill> QueryBills(int user_id, const QueryCriteria& criteria) const;

    // === 分类相关操作 ===

    bool AddCategory(const User& user, const Category& category);
    bool UpdateCategory(const User& user, const Category& category);
    bool DeleteCategory(const User& user, int category_id);
    std::vector<Category> GetCategories(const User& user) const;

    // === 预算相关操作 ===

    bool SetBudget(int user_id, const Budget& budget);
    std::shared_ptr<Budget> GetBudget(int user_id) const;

    // === 报表相关操作 ===

    Report GenerateReport(int user_id, const QueryCriteria& criteria,
                          Period period, ChartType chart_type);
    std::optional<Report> GetLastReport(int user_id) const;

private:
    // === 内部组件 ===
    std::shared_ptr<Storage> storage_;

    UserManager user_manager_;
    BillManager bill_manager_;
    BudgetManager budget_manager_;
    CategoryManager category_manager_;
    std::unique_ptr<ReportManager> report_manager_;

    // === 内部逻辑 ===
    bool CheckBudgetBeforeAdd(int user_id, const Bill& bill);
};

}  // namespace accounting

#endif  // ACCOUNTING_CORE_ACCOUNT_MANAGER_H_
