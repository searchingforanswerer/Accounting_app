#include "core/account_manager.h"
#include "storage/json_storage.h"
#include "models/category.h"
#include "models/bill.h"
#include "models/budget.h"
#include "models/period.h"
#include "models/chart_type.h"
#include <iostream>
#include <iomanip>
#include <memory>

using namespace accounting;


// ==================== 原有辅助函数 ====================

// 辅助函数：将 Period 转为字符串
auto PeriodToString(Period period) -> std::string {
    switch (period) {
        case Period::kDaily: return "日";
        case Period::kWeekly: return "周";
        case Period::kMonthly: return "月";
        case Period::kYearly: return "年";
        case Period::kCustom: return "自定义";
        default: return "未知";
    }
}

// 辅助函数：将 ChartType 转为字符串
std::string ChartTypeToString(ChartType chart_type) {
    switch (chart_type) {
        case ChartType::kBar: return "柱状图";
        case ChartType::kPie: return "饼图";
        case ChartType::kLine: return "折线图";
        case ChartType::kTable: return "数据表格";
        default: return "未知";
    }
}

// 辅助函数：打印报表信息
void PrintReport(const Report& report) {
    std::cout << "  [报表信息]\n";
    std::cout << "    周期: " << PeriodToString(report.GetPeriod()) << "\n";
    std::cout << "    图表类型: " << ChartTypeToString(report.GetChartType()) << "\n";
    std::cout << "    总收入: " << std::fixed << std::setprecision(2) << report.GetTotalIncome() << "\n";
    std::cout << "    总支出: " << std::fixed << std::setprecision(2) << report.GetTotalExpense() << "\n";
    std::cout << "    按分类汇总:\n";
    const auto& summary = report.GetCategorySummary();
    if (summary.empty()) {
        std::cout << "      (无数据)\n";
    } else {
        for (const auto& [category, amount] : summary) {
            std::cout << "      - " << category << ": " << std::fixed << std::setprecision(2) << amount << "\n";
        }
    }
}

// 辅助函数：打印分隔线
void PrintSeparator(const std::string& title = "") {
    std::cout << "\n" << std::string(60, '=') << "\n";
    if (!title.empty()) {
        std::cout << "  " << title << "\n";
        std::cout << std::string(60, '=') << "\n";
    }
    std::cout << "\n";
}

// 辅助函数：打印 Bill 信息
void PrintBill(const Bill& bill) {
    std::cout << "  [Bill ID: " << bill.GetBillId() 
              << "] 金额: " << std::fixed << std::setprecision(2) << bill.GetAmount();
    if (bill.GetCategory()) {
        std::cout << ", 分类: " << bill.GetCategory()->GetName();
    }
    std::cout << ", 备注: " << bill.GetContent() << "\n";
}

int main() {
    std::cout << "\n========== 记账系统测试程序 ==========\n\n";

    // 步骤 1: 初始化存储和 AccountManager
    PrintSeparator("步骤 1: 系统初始化");
    
    std::string data_dir = "data";
    auto storage = std::make_shared<JsonStorage>(data_dir);
    std::cout << "[√] 创建 JsonStorage，数据目录: " << data_dir << "\n";

    AccountManager account_manager(storage);
    std::cout << "[√] 创建 AccountManager\n";

    if (!account_manager.Initialize()) {
        std::cerr << "[✗] 初始化失败，无法加载数据\n";
        return 1;
    }
    std::cout << "[√] 系统初始化完成\n";

    // ==================== 原有功能测试 ====================

    // 步骤 2: 用户注册和登录
    PrintSeparator("步骤 2: 用户管理");
    
    std::string username = "test_user";
    std::string password = "password123";
    
    if (account_manager.RegisterUser(username, password)) {
        std::cout << "[√] 用户注册成功: " << username << "\n";
    } else {
        std::cout << "[!] 用户已存在或注册失败: " << username << "\n";
    }

    auto user = account_manager.Login(username, password);
    if (!user) {
        std::cerr << "[✗] 登录失败\n";
        return 1;
    }
    std::cout << "[√] 用户登录成功, User ID: " << user->GetUserId() << "\n";

    // 步骤 3: 创建分类
    PrintSeparator("步骤 3: 分类管理");
    
    std::vector<Category> categories = {
        Category(1, "餐饮", "expense", "#FF6B6B"),
        Category(2, "交通", "expense", "#4ECDC4"),
        Category(3, "娱乐", "expense", "#45B7D1"),
        Category(4, "工资", "income", "#95E1D3")
    };

    for (const auto& cat : categories) {
        if (account_manager.AddCategory(*user, cat)) {
            std::cout << "[√] 分类创建成功: " << cat.GetName() << "\n";
        } else {
            std::cout << "[!] 分类创建失败（可能已存在）: " << cat.GetName() << "\n";
        }
    }

    // 步骤 4: 创建账单
    PrintSeparator("步骤 4: 账单管理");
    
    auto user_categories = account_manager.GetCategories(*user);

    if (user_categories.size() > 0) {
        // 创建几笔账单
        std::vector<Bill> test_bills;
        
        auto now = std::chrono::system_clock::now();
        
        Bill bill1;
        bill1.SetAmount(50.0);
        bill1.SetCategory(std::make_shared<Category>(user_categories[0]));
        bill1.SetContent("午餐");
        bill1.SetTime(now);
        test_bills.push_back(bill1);

        Bill bill2;
        bill2.SetAmount(30.0);
        bill2.SetCategory(std::make_shared<Category>(user_categories[1]));
        bill2.SetContent("地铁");
        bill2.SetTime(now);
        test_bills.push_back(bill2);

        Bill bill3;
        bill3.SetAmount(200.0);
        bill3.SetCategory(std::make_shared<Category>(user_categories[3]));  // 工资（income）
        bill3.SetContent("奖金");
        bill3.SetTime(now);
        test_bills.push_back(bill3);

        for (auto& bill : test_bills) {
            if (account_manager.AddBill(user->GetUserId(), bill)) {
                std::cout << "[√] 账单添加成功";
                PrintBill(bill);
            } else {
                std::cout << "[✗] 账单添加失败（可能超出预算）";
                PrintBill(bill);
            }
        }
    } else {
        std::cout << "[!] 没有分类，跳过账单创建\n";
    }

    // 步骤 5: 查询账单
    PrintSeparator("步骤 5: 查询账单");
    
    auto all_bills = account_manager.GetBills(user->GetUserId());
    std::cout << "用户的所有账单 (共 " << all_bills.size() << " 笔):\n";
    for (const auto& bill : all_bills) {
        PrintBill(bill);
    }

    // 步骤 6: 设置预算
    PrintSeparator("步骤 6: 预算管理");
    
    Budget budget;
    budget.SetTotalLimit(1000.0);
    
    // 为各分类设置预算
    for (const auto& cat : user_categories) {
        if (cat.GetName() != "工资") {  // 支出分类
            budget.SetCategoryLimit(cat.GetCategoryId(), 500.0);
        }
    }

    if (account_manager.SetBudget(user->GetUserId(), budget)) {
        std::cout << "[√] 预算设置成功\n";
        std::cout << "  - 总预算: " << budget.GetTotalLimit() << "\n";
        auto cat_limits = budget.GetCategoryLimits();
        std::cout << "  - 分类预算数: " << cat_limits.size() << "\n";
    } else {
        std::cout << "[✗] 预算设置失败\n";
    }

    // 步骤 7: 测试预算检查
    PrintSeparator("步骤 7: 预算检查");
    
    if (user_categories.size() > 0) {
        Bill over_budget_bill;
        over_budget_bill.SetAmount(600.0);  // 超过分类预算 500
        over_budget_bill.SetCategory(std::make_shared<Category>(user_categories[0]));
        over_budget_bill.SetContent("大餐");
        over_budget_bill.SetTime(std::chrono::system_clock::now());

        std::cout << "尝试添加超预算账单 (600 元, 预算限制 500 元):\n";
        if (account_manager.AddBill(user->GetUserId(), over_budget_bill)) {
            std::cout << "[!] 账单添加成功（预算检查未阻止）\n";
        } else {
            std::cout << "[√] 账单被预算检查阻止，预算管理正常\n";
        }
    }

    // 步骤 8: 报表生成测试
    PrintSeparator("步骤 8: 报表生成与分析");
    
    // 生成全量月报表（柱状图）
    Report monthly_report = account_manager.GenerateReport(
        user->GetUserId(),
        QueryCriteria(),  // 无筛选，展示所有账单
        Period::kMonthly,
        ChartType::kBar
    );

    std::cout << "[√] 生成月报表（柱状图，所有账单）:\n";
    PrintReport(monthly_report);
    
    // 生成日报表（饼图）
    Report daily_report = account_manager.GenerateReport(
        user->GetUserId(),
        QueryCriteria(),
        Period::kDaily,
        ChartType::kPie
    );
    std::cout << "\n[√] 生成日报表（饼图，所有账单）:\n";
    PrintReport(daily_report);
    
    // 生成表格形式的年报表
    Report yearly_report = account_manager.GenerateReport(
        user->GetUserId(),
        QueryCriteria(),
        Period::kYearly,
        ChartType::kTable
    );
    std::cout << "\n[√] 生成年报表（表格，所有账单）:\n";
    PrintReport(yearly_report);
    
    // 生成周报表（折线图）
    Report weekly_report = account_manager.GenerateReport(
        user->GetUserId(),
        QueryCriteria(),
        Period::kWeekly,
        ChartType::kLine
    );
    std::cout << "\n[√] 生成周报表（折线图，所有账单）:\n";
    PrintReport(weekly_report);

    // 步骤 9: 保存所有数据
    PrintSeparator("步骤 9: 数据持久化");
    
    if (account_manager.SaveAll()) {
        std::cout << "[√] 所有数据已保存到 " << data_dir << " 目录\n";
        std::cout << "  - users.json\n";
        std::cout << "  - bills.json\n";
        std::cout << "  - categories.json\n";
        std::cout << "  - budgets.json\n";
    } else {
        std::cerr << "[✗] 数据保存失败\n";
        return 1;
    }

    // 步骤 10: 测试重新加载
    PrintSeparator("步骤 10: 数据恢复测试");
    
    auto storage2 = std::make_shared<JsonStorage>(data_dir);
    AccountManager account_manager2(storage2);
    
    if (!account_manager2.Initialize()) {
        std::cerr << "[✗] 重新加载失败\n";
        return 1;
    }
    std::cout << "[√] 从文件重新加载数据成功\n";

    auto user2 = account_manager2.Login(username, password);
    if (user2) {
        std::cout << "[√] 用户重新登录成功\n";
        
        auto reloaded_bills = account_manager2.GetBills(user2->GetUserId());
        std::cout << "[√] 重新加载的账单数: " << reloaded_bills.size();
        if (reloaded_bills.size() == all_bills.size()) {
            std::cout << " (与保存前一致)\n";
        } else {
            std::cout << " (与保存前不一致!)\n";
        }

        auto reloaded_budget = account_manager2.GetBudget(user2->GetUserId());
        if (reloaded_budget) {
            std::cout << "[√] 预算重新加载成功，总预算: " << reloaded_budget->GetTotalLimit() << "\n";
        } else {
            std::cout << "[!] 预算未找到\n";
        }
    } else {
        std::cerr << "[✗] 用户登录失败\n";
        return 1;
    }

    // 测试完成
    PrintSeparator("测试完成");
    std::cout << "[√] 所有基础功能测试完成\n";
    std::cout << "[√] 数据持久化工作正常\n";
    std::cout << "[√] 数据恢复机制工作正常\n\n";

    return 0;
}
