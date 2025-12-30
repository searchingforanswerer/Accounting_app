#ifndef ACCOUNTING_CORE_ACCOUNT_MANAGER_H_
#define ACCOUNTING_CORE_ACCOUNT_MANAGER_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <chrono>

#include "storage/storage.h"
#include "managers/user_manager.h"
#include "managers/bill_manager.h"
#include "managers/budget_manager.h"
#include "managers/category_manager.h"
#include "managers/report_manager.h"
#include "core/operation_result.h"
#include "core/query_result_types.h"

namespace accounting {

/**
 * @brief AccountManager 是系统的核心调度器（业务外观 Facade）。
 *
 * 它负责协调所有 Manager（User/Bill/Budget/Category/Report），
 * 提供统一的接口供上层 UI（CLI/Web/GUI）调用。
 * 同时封装业务逻辑，如账单添加、预算检查、报表生成等。
 * 
 * 接口设计遵循以下原则：
 * 1. 返回值统一使用 OperationResult<T>，携带错误码和错误信息
 * 2. 提供验证接口（Validate*），UI 可在用户输入后立即验证
 * 3. 提供高效查询接口（按日期、分类、分页等）
 * 4. 提供预算分析接口（查询预算状态、影响分析等）
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

    // ========== 第一阶段：带错误处理的用户相关操作 ==========

    /**
     * @brief 注册新用户（带详细错误信息）
     * @param username 用户名
     * @param password 密码
     * @return 包含新用户对象的结果，或错误信息
     */
    OperationResult<std::shared_ptr<User>> RegisterUserEx(const std::string& username,
                                                          const std::string& password);

    /**
     * @brief 用户登录（带详细错误信息）
     * @param username 用户名
     * @param password 密码
     * @return 包含已登录用户对象的结果，或错误信息
     */
    OperationResult<std::shared_ptr<User>> LoginEx(const std::string& username,
                                                    const std::string& password);

    // 原有的 bool 版本保留以保持兼容性
    bool RegisterUser(const std::string& username, const std::string& password);
    std::shared_ptr<User> Login(const std::string& username, const std::string& password);

    // ========== 第一阶段：带错误处理的账单相关操作 ==========

    /**
     * @brief 添加账单（带详细错误信息）
     * @param user_id 用户 ID
     * @param bill 待添加的账单
     * @return 添加成功/失败的结果和错误信息
     */
    OperationResult<void> AddBillEx(int user_id, const Bill& bill);

    /**
     * @brief 更新账单（带详细错误信息）
     * @param user_id 用户 ID
     * @param bill 待更新的账单
     * @return 更新成功/失败的结果和错误信息
     */
    OperationResult<void> UpdateBillEx(int user_id, const Bill& bill);

    /**
     * @brief 删除账单（带详细错误信息）
     * @param user_id 用户 ID
     * @param bill_id 待删除的账单 ID
     * @return 删除成功/失败的结果和错误信息
     */
    OperationResult<void> DeleteBillEx(int user_id, int bill_id);

    // 原有的 bool 版本保留
    bool AddBill(int user_id, Bill bill);
    bool UpdateBill(int user_id, const Bill& bill);
    bool DeleteBill(int user_id, int bill_id);
    std::vector<Bill> GetBills(int user_id) const;
    std::vector<Bill> QueryBills(int user_id, const QueryCriteria& criteria) const;

    // ========== 第一阶段：带错误处理的分类相关操作 ==========

    /**
     * @brief 添加分类（带详细错误信息）
     * @param user 当前用户
     * @param category 待添加的分类
     * @return 添加成功/失败的结果和错误信息
     */
    OperationResult<void> AddCategoryEx(const User& user, const Category& category);

    /**
     * @brief 更新分类（带详细错误信息）
     * @param user 当前用户
     * @param category 待更新的分类
     * @return 更新成功/失败的结果和错误信息
     */
    OperationResult<void> UpdateCategoryEx(const User& user, const Category& category);

    /**
     * @brief 删除分类（带详细错误信息）
     * @param user 当前用户
     * @param category_id 待删除的分类 ID
     * @return 删除成功/失败的结果和错误信息
     */
    OperationResult<void> DeleteCategoryEx(const User& user, int category_id);

    // 原有的 bool 版本保留
    bool AddCategory(const User& user, const Category& category);
    bool UpdateCategory(const User& user, const Category& category);
    bool DeleteCategory(const User& user, int category_id);
    std::vector<Category> GetCategories(const User& user) const;

    // ========== 第一阶段：带错误处理的预算相关操作 ==========

    /**
     * @brief 设置预算（带详细错误信息）
     * @param user_id 用户 ID
     * @param budget 待设置的预算
     * @return 设置成功/失败的结果和错误信息
     */
    OperationResult<void> SetBudgetEx(int user_id, const Budget& budget);

    // 原有的 bool 版本保留
    bool SetBudget(int user_id, const Budget& budget);
    std::shared_ptr<Budget> GetBudget(int user_id) const;

    // ========== 第二阶段：数据验证接口 ==========

    /**
     * @brief 验证用户名和密码的有效性
     * @param username 用户名
     * @param password 密码
     * @return 验证结果，包含具体的验证错误信息
     */
    OperationResult<void> ValidateUserInput(const std::string& username,
                                           const std::string& password) const;

    /**
     * @brief 验证账单的有效性（金额、日期等）
     * @param bill 待验证的账单
     * @return 验证结果
     */
    OperationResult<void> ValidateBill(const Bill& bill) const;

    /**
     * @brief 验证分类的有效性
     * @param user 当前用户
     * @param category 待验证的分类
     * @return 验证结果
     */
    OperationResult<void> ValidateCategory(const User& user,
                                          const Category& category) const;

    /**
     * @brief 验证预算的有效性
     * @param budget 待验证的预算
     * @return 验证结果
     */
    OperationResult<void> ValidateBudget(const Budget& budget) const;

    // ========== 第三阶段：高效查询接口 ==========

    /**
     * @brief 按日期范围查询账单
     * @param user_id 用户 ID
     * @param start_date 起始日期（YYYY-MM-DD 格式）
     * @param end_date 结束日期（YYYY-MM-DD 格式）
     * @return 符合条件的账单列表
     */
    std::vector<Bill> GetBillsByDateRange(int user_id,
                                         const std::string& start_date,
                                         const std::string& end_date) const;

    /**
     * @brief 按分类查询账单
     * @param user_id 用户 ID
     * @param category_id 分类 ID
     * @return 该分类下的所有账单
     */
    std::vector<Bill> GetBillsByCategory(int user_id, int category_id) const;

    /**
     * @brief 按分类和日期范围查询账单
     * @param user_id 用户 ID
     * @param category_id 分类 ID
     * @param start_date 起始日期（YYYY-MM-DD 格式）
     * @param end_date 结束日期（YYYY-MM-DD 格式）
     * @return 符合条件的账单列表
     */
    std::vector<Bill> GetBillsByCategoryAndDate(int user_id,
                                               int category_id,
                                               const std::string& start_date,
                                               const std::string& end_date) const;

    /**
     * @brief 获取分页的账单列表
     * @param user_id 用户 ID
     * @param page_number 页码（从 1 开始）
     * @param page_size 每页大小
     * @return 包含分页信息的账单列表
     */
    PagedResult<Bill> GetBillsPaged(int user_id, int page_number, int page_size) const;

    /**
     * @brief 统计指定分类在日期范围内的总支出
     * @param user_id 用户 ID
     * @param category_id 分类 ID
     * @param start_date 起始日期（YYYY-MM-DD 格式）
     * @param end_date 结束日期（YYYY-MM-DD 格式）
     * @return 总支出金额
     */
    double GetTotalExpenseByCategory(int user_id, int category_id,
                                     const std::string& start_date,
                                     const std::string& end_date) const;

    /**
     * @brief 统计用户在日期范围内的总支出
     * @param user_id 用户 ID
     * @param start_date 起始日期（YYYY-MM-DD 格式）
     * @param end_date 结束日期（YYYY-MM-DD 格式）
     * @return 总支出金额
     */
    double GetTotalExpense(int user_id,
                          const std::string& start_date,
                          const std::string& end_date) const;

    // ========== 第四阶段：预算分析接口 ==========

    /**
     * @brief 获取用户的总预算使用状态
     * @param user_id 用户 ID
     * @return 包含预算使用百分比、剩余预算等信息
     */
    BudgetStatus GetBudgetStatus(int user_id) const;

    /**
     * @brief 获取所有分类的预算使用状态
     * @param user_id 用户 ID
     * @return 每个分类的预算使用情况
     */
    std::vector<CategoryBudgetStatus> GetCategoryBudgetStatus(int user_id) const;

    /**
     * @brief 分析添加一笔账单对预算的影响
     * 
     * 该方法不会实际添加账单，仅用于预测影响，供 UI 显示警告信息
     * 
     * @param user_id 用户 ID
     * @param bill 待添加的账单
     * @return 添加该账单后的预算影响分析
     */
    BudgetImpact GetBudgetImpactIfAddBill(int user_id, const Bill& bill) const;

    // ========== 报表相关操作 ==========

    Report GenerateReport(int user_id, const QueryCriteria& criteria,
                          Period period, ChartType chart_type);
    std::optional<Report> GetLastReport(int user_id) const;

    // ========== 原有的便利方法 ==========

    // 检查在添加该账单前是否满足预算（公开包装，供 UI/CLI 检查使用）
    bool CanAddBill(int user_id, const Bill& bill) const;

    // === 公共日期/时间与日汇总工具（供 CLI/测试使用） ===
    /**
     * @brief 将 YYYY-MM-DD 字符串解析为 std::chrono::system_clock::time_point
     * @param date_str 日期字符串
     * @param out 解析后的 time_point（仅在返回 true 时有效）
     * @return 解析成功返回 true，否则返回 false
     */
    bool ParseDateStringToTimePoint(const std::string& date_str,
                             std::chrono::system_clock::time_point& out) const;

    /**
     * @brief 将日期和时间字符串解析为 time_point
     * @param date_str YYYY-MM-DD
     * @param time_str HH:MM 或 HH:MM:SS
     * @param out 解析结果
     * @return 成功返回 true
     */
    bool ParseDateTimeStringToTimePoint(const std::string& date_str,
                                        const std::string& time_str,
                                        std::chrono::system_clock::time_point& out) const;

    /**
     * @brief 获取指定日期的日汇总（收入与支出）
     * @param user_id 用户 ID
     * @param date_str 日期字符串 YYYY-MM-DD
     * @return pair(first=total_income, second=total_expense)
     */
    std::pair<double, double> GetDailySummary(int user_id, const std::string& date_str) const;

private:
    // === 内部组件 ===
    std::shared_ptr<Storage> storage_;

    UserManager user_manager_;
    BillManager bill_manager_;
    BudgetManager budget_manager_;
    CategoryManager category_manager_;
    std::unique_ptr<ReportManager> report_manager_;

    // === 内部逻辑 ===
    // 注意：此方法不修改对象状态，因此标记为 const，使得
    // 在 const 上下文（例如 CanAddBill）中可安全调用。
    bool CheckBudgetBeforeAdd(int user_id, const Bill& bill) const;

    // === 内部辅助方法 ===
    
    /**
     * @brief 解析日期字符串（YYYY-MM-DD 格式）
     * @param date_str 日期字符串
    * @return 解析成功返回 true，并将解析结果写入 out；否则返回 false
     */
    bool IsValidDateFormat(const std::string& date_str) const;

    /**
     * @brief 比较两个日期字符串
     * @param date1 第一个日期（YYYY-MM-DD）
     * @param date2 第二个日期（YYYY-MM-DD）
     * @return date1 <= date2 返回 true
     */
    bool IsDateLessOrEqual(const std::string& date1, const std::string& date2) const;
};

}  // namespace accounting

#endif  // ACCOUNTING_CORE_ACCOUNT_MANAGER_H_
