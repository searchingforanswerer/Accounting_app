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

// ==================== 明显的缺陷函数（未调用） ====================

// // BUG 1: 数组越界 - 固定访问越界索引
// void ArrayOutOfBoundsUncalled() {
//     int arr[5] = {1, 2, 3, 4, 5};
//     int value = arr[10];  // 明显越界
//     std::cout << value << "\n";
// }

// // BUG 2: 空指针解引用 - 明确设置为 nullptr 后解引用
// void NullPointerDerefUncalled() {
//     int* ptr = nullptr;
//     *ptr = 42;  // 明显的空指针解引用
// }

// // BUG 3: 除零错误
// void DivideByZeroUncalled() {
//     int a = 10;
//     int b = 0;
//     int result = a / b;  // 除零
//     std::cout << result << "\n";
// }

// // BUG 4: 使用未初始化的变量
// void UseUninitializedUncalled() {
//     int x;
//     int y = x + 10;  // x 未初始化
//     std::cout << y << "\n";
// }

// // BUG 5: 内存泄漏 - 分配后立即丢失指针
// void MemoryLeakUncalled() {
//     int* ptr = new int[100];
//     ptr = nullptr;  // 内存泄漏，原始指针丢失
// }

// // BUG 6: 越界写入
// void BufferOverflowWriteUncalled() {
//     char buffer[10];
//     strcpy(buffer, "This is a very long string that will overflow");  // 明显溢出
// }

// // BUG 7: Use after free
// void UseAfterFreeUncalled() {
//     int* p = new int(42);
//     delete p;
//     *p = 100;  // 使用已释放的内存
// }

// // BUG 8: Double free
// void DoubleFreeUncalled() {
//     int* p = new int(42);
//     delete p;
//     delete p;  // 重复释放
// }

// ==================== False Positive 诱饵函数 ====================

// // FALSE POSITIVE 1: 看似空指针解引用，但实际有检查（复杂控制流）
// void FalsePositive_NullCheck(int* ptr) {
//     // clang-tidy 可能因为复杂的控制流而误报
//     bool is_valid = (ptr != nullptr);
    
//     if (is_valid) {
//         int temp = *ptr;  // 实际安全，但工具可能误报
//         std::cout << "Value: " << temp << "\n";
//     }
// }

// // FALSE POSITIVE 2: 看似数组越界，但有运行时检查
// void FalsePositive_ArrayBounds(const std::vector<int>& vec, size_t index) {
//     // 工具可能不理解 vector 的动态边界检查
//     if (index < vec.size()) {
//         int value = vec[index];  // 安全，但可能被误报
//         std::cout << "Value: " << value << "\n";
//     }
// }

// // FALSE POSITIVE 3: 看似内存泄漏，但实际由智能指针管理
// std::shared_ptr<int> FalsePositive_SmartPointer() {
//     // 工具可能认为这是裸指针泄漏
//     int* raw = new int(100);
//     return std::shared_ptr<int>(raw);  // 实际由 shared_ptr 管理，不会泄漏
// }

// // FALSE POSITIVE 4: 看似 Use After Free，但实际是两个不同的对象
// void FalsePositive_DifferentObjects() {
//     int* p1 = new int(10);
//     int* p2 = new int(20);
    
//     delete p1;
    
//     // 工具可能混淆 p1 和 p2
//     int value = *p2;  // 安全，使用的是 p2 不是 p1
//     std::cout << value << "\n";
    
//     delete p2;
// }

// // FALSE POSITIVE 5: 看似未初始化，但实际通过函数初始化
// void InitializeValue(int& x) {
//     x = 42;
// }

// void FalsePositive_InitializedByFunction() {
//     int x;  // 看似未初始化
//     InitializeValue(x);  // 但通过函数初始化了
//     std::cout << "x = " << x << "\n";  // 安全，但工具可能误报
// }

// // FALSE POSITIVE 6: 看似整数溢出，但实际在安全范围内
// int FalsePositive_SafeMultiplication(int a, int b) {
//     // 工具可能认为任何乘法都有溢出风险
//     if (a > 0 && a < 100 && b > 0 && b < 100) {
//         return a * b;  // 实际安全（最大 9900）
//     }
//     return 0;
// }

// // FALSE POSITIVE 7: 看似悬空指针，但使用了引用
// int& FalsePositive_ReferenceNotPointer() {
//     static int value = 100;  // static 变量，不会被销毁
//     return value;  // 返回引用是安全的
// }

// // FALSE POSITIVE 8: 看似条件竞争，但实际是单线程
// class FalsePositive_SingleThreaded {
// private:
//     int counter = 0;
// public:
//     void increment() {
//         counter++;  // 单线程下安全，但工具可能警告缺少互斥锁
//     }
//     int get() const { return counter; }
// };

// // FALSE POSITIVE 9: 看似缓冲区溢出，但有长度检查
// void FalsePositive_BoundedCopy(const char* src) {
//     char buffer[100];
//     size_t len = std::strlen(src);
    
//     if (len < 100) {  // 有长度检查
//         std::strcpy(buffer, src);  // 安全，但工具可能因为 strcpy 而警告
//         std::cout << buffer << "\n";
//     }
// }

// // FALSE POSITIVE 10: 看似双重释放，但有状态跟踪
// class FalsePositive_ResourceManager {
// private:
//     int* data;
//     bool is_owned;
    
// public:
//     FalsePositive_ResourceManager(int* ptr, bool own) : data(ptr), is_owned(own) {}
    
//     ~FalsePositive_ResourceManager() {
//         if (is_owned && data) {  // 有检查，不会双重释放
//             delete data;
//             data = nullptr;
//         }
//     }
// };

// ==================== 被调用的缺陷函数 ====================

// 越界访问
// 数组越界
// CWE-125 Out-of-bounds Read
// CWE-787 Out-of-bounds Write
void ArrayOutOfBoundsCalled() {
    int arr[5] = {1, 2, 3, 4, 5};
    for (int i = 0; i <= 5; i++) {
        std::cout << arr[i] << " ";
    }
    arr[5] = 6;
    std::cout << arr[5] << " ";
    std::cout << "\n";
}
// 缓冲区读取越界
// CWE-787 Out-of-bounds Write
void BufferOverreadCalled(const char* str, size_t len) {
    char buffer[10];
    // len 可能大于 10
    for (size_t i = 0; i < len; i++) {
        buffer[i] = str[i];
    }
}
// 越界访问 vector
// CWE-125 Out-of-bounds Read
void VectorOutOfBoundsCalled(std::vector<int>& vec) {
    // 未检查大小就访问
    int value = vec[100];  // 如果 vec 大小 < 100，越界
    std::cout << value << "\n";
}

// 空指针解引用
// CWE-476 NULL Pointer Deference
// 空指针未检查直接解引用
void NullPointerDerefCalled(int* ptr) {
    // 未检查 ptr 是否为 nullptr
    std::cout << "Value: " << *ptr << "\n";
}
// 直接空指针解引用
void DirectNullPointerDereference() {
    int* ptr = nullptr;
    
    // 尝试读取空指针
    int value = *ptr;  // BUG: 空指针解引用
    std::cout << "Value: " << value << "\n";
    
    // 尝试写入空指针
    *ptr = 42;  // BUG: 空指针解引用
}

// 整数溢出
// CWE-190 Integer OverFlow
int IntegerOverflowCalled(int a, int b) {
    return a * b;  // 无溢出检查
}
// CWE-191 Integer Underflow
int SignedUnderflowCalled(int a) {
    return a - 1000000000;  // 如果 a 很小，可能下溢
}
// CWE-190
void TestIntegerOverflowExplicitly() {
    int large_a = 1000000;
    int large_b = 1000000;
    int product = large_a * large_b;
    std::cout << "1000000 * 1000000 = " << product << "\n";
}

// 内存释放相关
// CWE-401 Memory Leak
void MemoryLeakCalled() {
    int* leaked = new int[1000];
    // 从不释放
    std::cout << "分配了内存但未释放\n";
}
// CWE-415 Double Free
void DoubleFreeCalled() {
    int* ptr = new int(42);
    std::cout << "Value: " << *ptr << "\n";
    delete ptr;
    delete ptr;  // BUG: 双重释放
}
// CWE-416 Use After Free
void UseAfterFreeCalled() {
    int* ptr = new int(100);
    delete ptr;
    *ptr = 200;  // BUG: 在释放后使用指针
    std::cout << "After free: " << *ptr << "\n";
}

// 未初始化变量
// CWE-457 Use of Uninitialized Variable
void UninitializedVariableCalled() {
    int uninitialized;
    if (uninitialized > 0) {  // BUG: 使用未初始化的变量
        std::cout << "Positive\n";
    }
}

// 悬空指针（Dangling Pointer）
// CWE-562 Retrun of Stack Address
int* ReturnDanglingPointer() {
    int local = 42;
    return &local;  // 返回局部变量地址
}
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

    // ==================== 触发 False Positive 测试 ====================
    
    // PrintSeparator("False Positive 测试");

    // // FP1: 空指针检查（实际安全）
    // std::cout << "\n[FP测试] 空指针检查:\n";
    // int valid_value = 100;
    // FalsePositive_NullCheck(&valid_value);

    // // FP2: 数组边界检查（实际安全）
    // std::cout << "\n[FP测试] 数组边界:\n";
    // std::vector<int> test_vec = {1, 2, 3, 4, 5};
    // FalsePositive_ArrayBounds(test_vec, 2);

    // // FP3: 智能指针管理（实际安全）
    // std::cout << "\n[FP测试] 智能指针:\n";
    // auto smart_ptr = FalsePositive_SmartPointer();
    // std::cout << "Smart pointer value: " << *smart_ptr << "\n";

    // // FP4: 不同对象（实际安全）
    // std::cout << "\n[FP测试] 不同对象:\n";
    // FalsePositive_DifferentObjects();

    // // FP5: 函数初始化（实际安全）
    // std::cout << "\n[FP测试] 函数初始化:\n";
    // FalsePositive_InitializedByFunction();

    // // FP6: 安全的乘法（实际安全）
    // std::cout << "\n[FP测试] 安全乘法:\n";
    // int safe_result = FalsePositive_SafeMultiplication(10, 20);
    // std::cout << "Result: " << safe_result << "\n";

    // // FP7: 静态引用（实际安全）
    // std::cout << "\n[FP测试] 静态引用:\n";
    // int& ref = FalsePositive_ReferenceNotPointer();
    // std::cout << "Reference value: " << ref << "\n";

    // // FP8: 单线程计数器（实际安全）
    // std::cout << "\n[FP测试] 单线程计数:\n";
    // FalsePositive_SingleThreaded counter;
    // counter.increment();
    // std::cout << "Counter: " << counter.get() << "\n";

    // // FP9: 有界复制（实际安全）
    // std::cout << "\n[FP测试] 有界复制:\n";
    // FalsePositive_BoundedCopy("Short");

    // // FP10: 资源管理器（实际安全）
    // std::cout << "\n[FP测试] 资源管理:\n";
    // {
    //     int* managed = new int(50);
    //     FalsePositive_ResourceManager mgr(managed, true);
    //     // 析构时会正确释放
    // }

    // ==================== 触发真实缺陷 ====================
    
    PrintSeparator("触发缺陷测试");

    // 触发 BUG 9: 数组越界
    std::cout << "\n[测试] 数组越界访问:\n";
    ArrayOutOfBoundsCalled();

    // 触发 BUG 10: 空指针解引用
    std::cout << "\n[测试] 空指针解引用:\n";
    // NullPointerDerefCalled(nullptr);  // 取消注释会崩溃

    // 触发 BUG 11: 整数溢出
    std::cout << "\n[测试] 整数溢出:\n";
    int overflow_result = IntegerOverflowCalled(2000000000, 2);
    std::cout << "溢出结果: " << overflow_result << "\n";

    // 触发 BUG 12: 缓冲区越界写入
    std::cout << "\n[测试] 缓冲区越界写入:\n";
    const char* long_str = "This is a very long string";
    BufferOverreadCalled(long_str, 20);  // 缓冲区只有 10

    // 触发 BUG 13: 悬空指针
    std::cout << "\n[测试] 悬空指针:\n";
    int* dangling = ReturnDanglingPointer();
    // std::cout << *dangling << "\n";  // 取消注释会有未定义行为

    // 触发 BUG 14: 内存泄漏
    std::cout << "\n[测试] 内存泄漏:\n";
    MemoryLeakCalled();

    // 触发 BUG 15: vector 越界
    std::cout << "\n[测试] Vector 越界:\n";
    std::vector<int> small_vec = {1, 2, 3};
    // VectorOutOfBoundsCalled(small_vec);  // 取消注释会崩溃

    // 触发 BUG 16: 有符号整数下溢
    std::cout << "\n[测试] 整数下溢:\n";
    int underflow = SignedUnderflowCalled(-2000000000);
    std::cout << "下溢结果: " << underflow << "\n";

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
