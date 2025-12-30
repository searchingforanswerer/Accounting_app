#ifndef ACCOUNTING_CLI_CLI_H_
#define ACCOUNTING_CLI_CLI_H_

#include <memory>
#include <string>
#include "core/account_manager.h"

namespace accounting {

/**
 * @brief 终端交互式记账系统
 * 
 * 提供菜单驱动的用户界面，支持用户登录、账单管理、预算管理、报表生成等功能。
 */
class CLI {
public:
    /**
     * @brief 初始化 CLI，创建 AccountManager 并加载数据
     * @param data_dir 数据存储目录
     * @return true 初始化成功，false 初始化失败
     */
    bool Initialize(const std::string& data_dir = "data");

    /**
     * @brief 启动交互式主循环
     */
    void Run();

private:
    // 用户会话相关
    std::shared_ptr<User> current_user_;
    std::shared_ptr<AccountManager> account_manager_;

    // 主菜单
    void ShowMainMenu();
    int GetMenuChoice(int max_choice);

    // 用户认证
    void HandleUserAuth();
    void ShowAuthMenu();

    // 账户管理
    void HandleAccountMenu();
    void ShowAccountMenu();
    void RegisterNewUser();
    void LoginUser();
    void LogoutUser();

    // 分类管理
    void HandleCategoryMenu();
    void ShowCategoryMenu();
    void AddCategory();
    void ViewCategories();
    void DeleteCategory();

    // 账单管理
    void HandleBillMenu();
    void ShowBillMenu();
    void AddBill();
    void ViewBills();
    void UpdateBill();
    void DeleteBill();
    void QueryBills();

    // 预算管理
    void HandleBudgetMenu();
    void ShowBudgetMenu();
    void SetBudget();
    void ViewBudget();

    // 报表生成
    void HandleReportMenu();
    void ShowReportMenu();
    void GenerateReport();

    // 数据持久化
    void SaveData();

    // 辅助函数
    std::string GetUserInput(const std::string& prompt);
    double GetDoubleInput(const std::string& prompt);
    int GetIntInput(const std::string& prompt);
    void PrintSeparator(const std::string& title = "");
    void PrintSuccess(const std::string& message);
    void PrintError(const std::string& message);
    void PrintInfo(const std::string& message);
    void Pause();

    // 验证和转换
    Period GetPeriodFromUser();
    ChartType GetChartTypeFromUser();
};

}  // namespace accounting

#endif  // ACCOUNTING_CLI_CLI_H_
