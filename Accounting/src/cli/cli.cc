#include "cli/cli.h"
#include "storage/json_storage.h"
#include "models/period.h"
#include "models/chart_type.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <vector>

namespace accounting {

bool CLI::Initialize(const std::string& data_dir) {
    try {
        auto storage = std::make_shared<JsonStorage>(data_dir);
        account_manager_ = std::make_shared<AccountManager>(storage);
        
        if (!account_manager_->Initialize()) {
            PrintError("系统初始化失败，无法加载数据");
            return false;
        }
        
        PrintSuccess("系统初始化成功");
        return true;
    } catch (const std::exception& e) {
        PrintError(std::string("初始化异常: ") + e.what());
        return false;
    }
}

void CLI::Run() {
    PrintSeparator("欢迎使用记账系统");
    
    while (true) {
        if (!current_user_) {
            HandleUserAuth();
        } else {
            ShowMainMenu();
        }
    }
}

// ==================== 主菜单 ====================
void CLI::ShowMainMenu() {
    PrintSeparator("主菜单 [已登录: " + current_user_->GetUsername() + "]");
    
    std::cout << "\n  1. 账户管理\n";
    std::cout << "  2. 分类管理\n";
    std::cout << "  3. 账单管理\n";
    std::cout << "  4. 预算管理\n";
    std::cout << "  5. 报表生成\n";
    std::cout << "  6. 保存数据\n";
    std::cout << "  0. 退出系统\n\n";
    
    int choice = GetMenuChoice(6);
    
    switch (choice) {
        case 1: HandleAccountMenu(); break;
        case 2: HandleCategoryMenu(); break;
        case 3: HandleBillMenu(); break;
        case 4: HandleBudgetMenu(); break;
        case 5: HandleReportMenu(); break;
        case 6: SaveData(); break;
        case 0:
            SaveData();
            PrintInfo("感谢使用记账系统，再见！");
            exit(0);
        default:
            PrintError("无效选择");
    }
}

// ==================== 用户认证 ====================
void CLI::HandleUserAuth() {
    ShowAuthMenu();
    
    int choice = GetMenuChoice(2);
    
    switch (choice) {
        case 1: RegisterNewUser(); break;
        case 2: LoginUser(); break;
        case 0:
            PrintInfo("退出系统");
            exit(0);
        default:
            PrintError("无效选择");
    }
}

void CLI::ShowAuthMenu() {
    PrintSeparator("用户认证");
    
    std::cout << "\n  1. 注册新用户\n";
    std::cout << "  2. 登录\n";
    std::cout << "  0. 退出\n\n";
}

void CLI::RegisterNewUser() {
    PrintSeparator("用户注册");
    
    std::string username = GetUserInput("请输入用户名: ");
    std::string password = GetUserInput("请输入密码: ");
    
    if (account_manager_->RegisterUser(username, password)) {
        PrintSuccess("用户 [" + username + "] 注册成功");
        // 自动登录
        current_user_ = account_manager_->Login(username, password);
    } else {
        PrintError("注册失败，用户可能已存在");
    }
    
    Pause();
}

void CLI::LoginUser() {
    PrintSeparator("用户登录");
    
    std::string username = GetUserInput("请输入用户名: ");
    std::string password = GetUserInput("请输入密码: ");
    
    current_user_ = account_manager_->Login(username, password);
    
    if (current_user_) {
        PrintSuccess("登录成功，欢迎 " + username + "！");
    } else {
        PrintError("登录失败，用户名或密码错误");
    }
    
    Pause();
}

void CLI::LogoutUser() {
    if (current_user_) {
        PrintInfo("用户 [" + current_user_->GetUsername() + "] 已登出");
        current_user_ = nullptr;
    }
}

// ==================== 账户管理 ====================
void CLI::HandleAccountMenu() {
    ShowAccountMenu();
    
    int choice = GetMenuChoice(1);
    
    switch (choice) {
        case 1: LogoutUser(); break;
        case 0: break;
        default:
            PrintError("无效选择");
    }
}

void CLI::ShowAccountMenu() {
    PrintSeparator("账户管理");
    
    if (current_user_) {
        std::cout << "\n  当前用户: " << current_user_->GetUsername() << "\n";
        std::cout << "  用户ID: " << current_user_->GetUserId() << "\n\n";
        
        std::cout << "  1. 登出\n";
        std::cout << "  0. 返回\n\n";
    }
}

// ==================== 分类管理 ====================
void CLI::HandleCategoryMenu() {
    while (true) {
        ShowCategoryMenu();
        
        int choice = GetMenuChoice(3);
        
        switch (choice) {
            case 1: AddCategory(); break;
            case 2: ViewCategories(); break;
            case 3: DeleteCategory(); break;
            case 0: return;
            default:
                PrintError("无效选择");
        }
    }
}

void CLI::ShowCategoryMenu() {
    PrintSeparator("分类管理");
    
    std::cout << "\n  1. 添加分类\n";
    std::cout << "  2. 查看分类\n";
    std::cout << "  3. 删除分类\n";
    std::cout << "  0. 返回\n\n";
}

void CLI::AddCategory() {
    PrintSeparator("添加分类");
    
    std::string name = GetUserInput("分类名称: ");
    std::string type = GetUserInput("分类类型 (income/expense): ");
    std::string color = GetUserInput("分类颜色 (如 #FF5733): ");
    
    Category cat(0, name, type, color);  // ID 由系统生成
    
    if (account_manager_->AddCategory(*current_user_, cat)) {
        PrintSuccess("分类 [" + name + "] 添加成功");
    } else {
        PrintError("分类添加失败，可能名称重复");
    }
    
    Pause();
}

void CLI::ViewCategories() {
    PrintSeparator("分类列表");
    
    auto categories = account_manager_->GetCategories(*current_user_);
    
    if (categories.empty()) {
        PrintInfo("当前没有分类");
    } else {
        std::cout << "\n";
        for (const auto& cat : categories) {
            std::cout << "  [ID: " << cat.GetCategoryId() << "] "
                      << cat.GetName() << " (" << cat.GetType() << ") "
                      << cat.GetColor() << "\n";
        }
        std::cout << "\n";
    }
    
    Pause();
}

void CLI::DeleteCategory() {
    PrintSeparator("删除分类");
    
    ViewCategories();  // 先显示分类列表
    
    int category_id = GetIntInput("输入要删除的分类ID: ");
    
    if (account_manager_->DeleteCategory(*current_user_, category_id)) {
        PrintSuccess("分类删除成功");
    } else {
        PrintError("分类删除失败");
    }
    
    Pause();
}

// ==================== 账单管理 ====================
void CLI::HandleBillMenu() {
    while (true) {
        ShowBillMenu();
        
        int choice = GetMenuChoice(5);
        
        switch (choice) {
            case 1: AddBill(); break;
            case 2: ViewBills(); break;
            case 3: UpdateBill(); break;
            case 4: DeleteBill(); break;
            case 5: QueryBills(); break;
            case 0: return;
            default:
                PrintError("无效选择");
        }
    }
}

void CLI::ShowBillMenu() {
    PrintSeparator("账单管理");
    
    std::cout << "\n  1. 添加账单\n";
    std::cout << "  2. 查看所有账单\n";
    std::cout << "  3. 修改账单\n";
    std::cout << "  4. 删除账单\n";
    std::cout << "  5. 按条件查询\n";
    std::cout << "  0. 返回\n\n";
}

void CLI::AddBill() {
    PrintSeparator("添加账单");
    
    auto categories = account_manager_->GetCategories(*current_user_);
    if (categories.empty()) {
        PrintError("请先添加分类");
        Pause();
        return;
    }
    
    double amount = GetDoubleInput("金额: ");
    
    std::cout << "\n可用分类:\n";
    for (size_t i = 0; i < categories.size(); ++i) {
        std::cout << "  [" << i+1 << "] " << categories[i].GetName() << "\n";
    }
    
    int cat_choice = GetIntInput("选择分类 (序号): ");
    if (cat_choice < 1 || cat_choice > (int)categories.size()) {
        PrintError("无效的分类选择");
        Pause();
        return;
    }
    
    std::string content = GetUserInput("备注: ");
    
    Bill bill;
    bill.SetAmount(amount);
    auto chosen_category = std::make_shared<Category>(categories[cat_choice - 1]);
    bill.SetCategory(chosen_category);
    bill.SetContent(content);
    bill.SetTime(std::chrono::system_clock::now());

    // 先检查是否会超出预算
    bool within = account_manager_->CanAddBill(current_user_->GetUserId(), bill);
    if (within) {
        if (account_manager_->AddBill(current_user_->GetUserId(), bill)) {
            PrintSuccess("账单添加成功");
        } else {
            PrintError("账单添加失败");
        }
        Pause();
        return;
    }

    // 超出预算，给用户选项：忽略（将把分类预算提高到账单金额）、提高分类预算、提高总预算、取消
    PrintError("该账单可能超出预算");
    std::cout << "请选择处理方式:\n";
    std::cout << "  1. 忽略预算并添加（将自动把该分类预算提高到账单金额）\n";
    std::cout << "  2. 提高该分类预算后添加\n";
    std::cout << "  3. 提高总预算后添加\n";
    std::cout << "  0. 取消\n";

    int choice = GetIntInput("选择 (0-3): ");

    if (choice == 0) {
        PrintInfo("已取消添加账单");
        Pause();
        return;
    }

    // 取得当前预算（若不存在则新建）
    auto cur_budget_ptr = account_manager_->GetBudget(current_user_->GetUserId());
    Budget new_budget;
    if (cur_budget_ptr) new_budget = *cur_budget_ptr;

    int cat_id = chosen_category->GetCategoryId();

    if (choice == 1) {
        // 忽略：把分类预算设为至少账单金额
        double new_limit = std::max(new_budget.GetCategoryLimit(cat_id), amount);
        new_budget.SetCategoryLimit(cat_id, new_limit);
        if (!account_manager_->SetBudget(current_user_->GetUserId(), new_budget)) {
            PrintError("提高分类预算失败，无法添加账单");
            Pause();
            return;
        }
        if (account_manager_->AddBill(current_user_->GetUserId(), bill)) {
            PrintSuccess("已提高分类预算并添加账单");
        } else {
            PrintError("添加账单失败");
        }
        Pause();
        return;
    }

    if (choice == 2) {
        // 提高分类预算到用户输入的值
        double new_limit = GetDoubleInput("输入新的分类预算限额: ");
        new_budget.SetCategoryLimit(cat_id, new_limit);
        if (!account_manager_->SetBudget(current_user_->GetUserId(), new_budget)) {
            PrintError("设置分类预算失败");
            Pause();
            return;
        }
        if (account_manager_->AddBill(current_user_->GetUserId(), bill)) {
            PrintSuccess("已设置分类预算并添加账单");
        } else {
            PrintError("添加账单失败");
        }
        Pause();
        return;
    }

    if (choice == 3) {
        // 提高总预算
        double new_total = GetDoubleInput("输入新的总预算限额: ");
        new_budget.SetTotalLimit(new_total);
        if (!account_manager_->SetBudget(current_user_->GetUserId(), new_budget)) {
            PrintError("设置总预算失败");
            Pause();
            return;
        }
        if (account_manager_->AddBill(current_user_->GetUserId(), bill)) {
            PrintSuccess("已设置总预算并添加账单");
        } else {
            PrintError("添加账单失败");
        }
        Pause();
        return;
    }

    PrintError("无效选择，取消操作");
    Pause();
}

void CLI::ViewBills() {
    PrintSeparator("账单列表");
    
    auto bills = account_manager_->GetBills(current_user_->GetUserId());
    
    if (bills.empty()) {
        PrintInfo("当前没有账单");
    } else {
        std::cout << "\n";
        for (const auto& bill : bills) {
            std::cout << "  [ID: " << bill.GetBillId() << "] ";
            std::cout << std::fixed << std::setprecision(2) << bill.GetAmount();
            if (bill.GetCategory()) {
                std::cout << " - " << bill.GetCategory()->GetName();
            }
            std::cout << " - " << bill.GetContent() << "\n";
        }
        std::cout << "\n";
    }
    
    Pause();
}

void CLI::UpdateBill() {
    PrintSeparator("修改账单");
    
    ViewBills();
    
    int bill_id = GetIntInput("输入要修改的账单ID: ");
    double new_amount = GetDoubleInput("新金额: ");
    
    auto bills = account_manager_->GetBills(current_user_->GetUserId());
    for (auto& bill : bills) {
        if (bill.GetBillId() == bill_id) {
            bill.SetAmount(new_amount);
            if (account_manager_->UpdateBill(current_user_->GetUserId(), bill)) {
                PrintSuccess("账单修改成功");
            } else {
                PrintError("账单修改失败");
            }
            Pause();
            return;
        }
    }
    
    PrintError("未找到该账单");
    Pause();
}

void CLI::DeleteBill() {
    PrintSeparator("删除账单");
    
    ViewBills();
    
    int bill_id = GetIntInput("输入要删除的账单ID: ");
    
    if (account_manager_->DeleteBill(current_user_->GetUserId(), bill_id)) {
        PrintSuccess("账单删除成功");
    } else {
        PrintError("账单删除失败");
    }
    
    Pause();
}

void CLI::QueryBills() {
    PrintSeparator("按条件查询账单");
    
    PrintInfo("暂不支持高级查询，请使用'查看所有账单'功能");
    
    Pause();
}

// ==================== 预算管理 ====================
void CLI::HandleBudgetMenu() {
    while (true) {
        ShowBudgetMenu();
        
        int choice = GetMenuChoice(2);
        
        switch (choice) {
            case 1: SetBudget(); break;
            case 2: ViewBudget(); break;
            case 0: return;
            default:
                PrintError("无效选择");
        }
    }
}

void CLI::ShowBudgetMenu() {
    PrintSeparator("预算管理");
    
    std::cout << "\n  1. 设置预算\n";
    std::cout << "  2. 查看预算\n";
    std::cout << "  0. 返回\n\n";
}

void CLI::SetBudget() {
    PrintSeparator("设置预算");
    
    double total_limit = GetDoubleInput("总预算限额: ");
    
    Budget budget;
    budget.SetTotalLimit(total_limit);
    
    auto categories = account_manager_->GetCategories(*current_user_);
    
    if (!categories.empty()) {
        std::cout << "\n是否为各分类设置预算? (y/n): ";
        std::string resp;
        std::getline(std::cin, resp);
        
        if (resp == "y" || resp == "Y") {
            for (const auto& cat : categories) {
                double limit = GetDoubleInput("  [" + cat.GetName() + "] 预算: ");
                budget.SetCategoryLimit(cat.GetCategoryId(), limit);
            }
        }
    }
    
    if (account_manager_->SetBudget(current_user_->GetUserId(), budget)) {
        PrintSuccess("预算设置成功");
    } else {
        PrintError("预算设置失败");
    }
    
    Pause();
}

void CLI::ViewBudget() {
    PrintSeparator("预算信息");
    
    auto budget = account_manager_->GetBudget(current_user_->GetUserId());
    
    if (budget) {
        std::cout << "\n  总预算: " << std::fixed << std::setprecision(2) 
                  << budget->GetTotalLimit() << "\n";
        
        auto limits = budget->GetCategoryLimits();
        if (!limits.empty()) {
            std::cout << "  分类预算:\n";
            for (const auto& [cat_id, limit] : limits) {
                std::cout << "    [分类ID: " << cat_id << "] " 
                          << std::fixed << std::setprecision(2) << limit << "\n";
            }
        }
        std::cout << "\n";
    } else {
        PrintInfo("未设置预算");
    }
    
    Pause();
}

// ==================== 报表生成 ====================
void CLI::HandleReportMenu() {
    while (true) {
        ShowReportMenu();
        
        int choice = GetMenuChoice(1);
        
        switch (choice) {
            case 1: GenerateReport(); break;
            case 0: return;
            default:
                PrintError("无效选择");
        }
    }
}

void CLI::ShowReportMenu() {
    PrintSeparator("报表生成");
    
    std::cout << "\n  1. 生成报表\n";
    std::cout << "  0. 返回\n\n";
}

void CLI::GenerateReport() {
    PrintSeparator("生成报表");
    
    Period period = GetPeriodFromUser();
    ChartType chart_type = GetChartTypeFromUser();
    
    Report report = account_manager_->GenerateReport(
        current_user_->GetUserId(),
        QueryCriteria(),
        period,
        chart_type
    );
    
    PrintInfo("报表生成成功");
    
    std::cout << "\n  [报表摘要]\n";
    std::cout << "  总收入: " << std::fixed << std::setprecision(2) 
              << report.GetTotalIncome() << "\n";
    std::cout << "  总支出: " << std::fixed << std::setprecision(2) 
              << report.GetTotalExpense() << "\n";
    std::cout << "  按分类汇总:\n";
    
    const auto& summary = report.GetCategorySummary();
    if (summary.empty()) {
        std::cout << "    (无数据)\n";
    } else {
        for (const auto& [category, amount] : summary) {
            std::cout << "    - " << category << ": " << std::fixed 
                      << std::setprecision(2) << amount << "\n";
        }
    }
    std::cout << "\n";
    
    Pause();
}

// ==================== 数据持久化 ====================
void CLI::SaveData() {
    if (account_manager_->SaveAll()) {
        PrintSuccess("数据已保存");
    } else {
        PrintError("数据保存失败");
    }
}

// ==================== 辅助函数 ====================
std::string CLI::GetUserInput(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

double CLI::GetDoubleInput(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        double value;
        if (std::cin >> value) {
            std::cin.ignore();  // 清除换行符
            return value;
        } else {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            PrintError("请输入有效的数字");
        }
    }
}

int CLI::GetIntInput(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        int value;
        if (std::cin >> value) {
            std::cin.ignore();
            return value;
        } else {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            PrintError("请输入有效的整数");
        }
    }
}

void CLI::PrintSeparator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    if (!title.empty()) {
        std::cout << "  " << title << "\n";
        std::cout << std::string(60, '=') << "\n";
    }
    std::cout << "\n";
}

void CLI::PrintSuccess(const std::string& message) {
    std::cout << "\n  [✓] " << message << "\n\n";
}

void CLI::PrintError(const std::string& message) {
    std::cout << "\n  [✗] " << message << "\n\n";
}

void CLI::PrintInfo(const std::string& message) {
    std::cout << "\n  [i] " << message << "\n\n";
}

void CLI::Pause() {
    std::cout << "  按 Enter 键继续...";
    std::cin.ignore();
}

Period CLI::GetPeriodFromUser() {
    std::cout << "\n  选择报表周期:\n";
    std::cout << "    1. 日报\n";
    std::cout << "    2. 周报\n";
    std::cout << "    3. 月报\n";
    std::cout << "    4. 年报\n\n";
    
    int choice = GetIntInput("选择 (1-4): ");
    
    switch (choice) {
        case 1: return Period::kDaily;
        case 2: return Period::kWeekly;
        case 3: return Period::kMonthly;
        case 4: return Period::kYearly;
        default: return Period::kMonthly;
    }
}

ChartType CLI::GetChartTypeFromUser() {
    std::cout << "\n  选择图表类型:\n";
    std::cout << "    1. 柱状图\n";
    std::cout << "    2. 饼图\n";
    std::cout << "    3. 折线图\n";
    std::cout << "    4. 表格\n\n";
    
    int choice = GetIntInput("选择 (1-4): ");
    
    switch (choice) {
        case 1: return ChartType::kBar;
        case 2: return ChartType::kPie;
        case 3: return ChartType::kLine;
        case 4: return ChartType::kTable;
        default: return ChartType::kTable;
    }
}

int CLI::GetMenuChoice(int max_choice) {
    while (true) {
        std::cout << "  请选择 (0-" << max_choice << "): ";
        int choice;
        if (std::cin >> choice) {
            std::cin.ignore();
            if (choice >= 0 && choice <= max_choice) {
                return choice;
            }
        } else {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        }
        PrintError("无效的选择，请重试");
    }
}

}  // namespace accounting
