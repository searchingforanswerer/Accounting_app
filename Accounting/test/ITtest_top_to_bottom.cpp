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
#include <filesystem>
#include <gtest/gtest.h>
#include <iostream>

using namespace accounting;
namespace fs = std::filesystem;

// ==================== 测试夹具类 ====================
class AccountingSystemTest : public ::testing::Test {
protected:
    static int test_counter;
    std::string test_data_dir;
    std::shared_ptr<JsonStorage> storage;
    std::unique_ptr<AccountManager> account_manager;

    void SetUp() override {
        // 为每个测试创建独立的数据目录
        test_data_dir = "./test_data_" + std::to_string(test_counter++);
        
        // 清理旧的数据目录（如果存在）
        if (fs::exists(test_data_dir)) {
            fs::remove_all(test_data_dir);
        }
        
        // 创建新的数据目录
        fs::create_directories(test_data_dir);
        
        // 初始化存储和账户管理器
        storage = std::make_shared<JsonStorage>(test_data_dir);
        account_manager = std::make_unique<AccountManager>(storage);
    }

    void TearDown() override {
        // 清理测试数据目录
        if (fs::exists(test_data_dir)) {
            fs::remove_all(test_data_dir);
        }
    }
};

// 初始化静态计数器
int AccountingSystemTest::test_counter = 0;

// ==================== 自顶向下集成测试 ====================

// 步骤 1: 系统初始化
TEST_F(AccountingSystemTest, SystemInitialization) {
    EXPECT_TRUE(account_manager->Initialize()) << "初始化失败，无法加载数据";
}

// 步骤 2: 用户注册和登录
TEST_F(AccountingSystemTest, UserRegistrationAndLogin) {
    EXPECT_TRUE(account_manager->Initialize()) << "初始化失败，无法加载数据";

    std::string username = "test_user";
    std::string password = "password123";

    ASSERT_TRUE(account_manager->RegisterUser(username, password)) << "用户注册失败";
    auto user = account_manager->Login(username, password);
    ASSERT_TRUE(user) << "用户登录失败";
    EXPECT_EQ(user->GetUsername(), username) << "登录的用户名不正确";
}

// 步骤 3: 分类管理
TEST_F(AccountingSystemTest, CategoryManagement) {
    EXPECT_TRUE(account_manager->Initialize()) << "初始化失败，无法加载数据";

    std::string username = "test_user";
    std::string password = "password123";
    account_manager->RegisterUser(username, password);
    auto user = account_manager->Login(username, password);

    // 创建分类
    Category category1(1, "餐饮", "expense", "#FF6B6B");
    Category category2(2, "交通", "expense", "#4ECDC4");

    ASSERT_TRUE(account_manager->AddCategory(*user, category1)) << "分类 '餐饮' 创建失败";
    ASSERT_TRUE(account_manager->AddCategory(*user, category2)) << "分类 '交通' 创建失败";

    auto categories = account_manager->GetCategories(*user);
    EXPECT_EQ(categories.size(), 2) << "分类数量不正确";
    EXPECT_EQ(categories[0].GetName(), "餐饮") << "分类 '餐饮' 不正确";
    EXPECT_EQ(categories[1].GetName(), "交通") << "分类 '交通' 不正确";
}

// 步骤 4: 账单管理
TEST_F(AccountingSystemTest, BillManagement) {
    EXPECT_TRUE(account_manager->Initialize()) << "初始化失败，无法加载数据";

    std::string username = "test_user";
    std::string password = "password123";
    account_manager->RegisterUser(username, password);
    auto user = account_manager->Login(username, password);

    // 创建分类
    Category category1(1, "餐饮", "expense", "#FF6B6B");
    account_manager->AddCategory(*user, category1);

    // 创建账单
    Bill bill;
    bill.SetAmount(50.0);
    bill.SetCategory(std::make_shared<Category>(category1));
    bill.SetContent("午餐");
    bill.SetTime(std::chrono::system_clock::now());

    ASSERT_TRUE(account_manager->AddBill(user->GetUserId(), bill)) << "账单添加失败";

    auto bills = account_manager->GetBills(user->GetUserId());
    EXPECT_EQ(bills.size(), 1) << "账单数量不正确";
    EXPECT_EQ(bills[0].GetContent(), "午餐") << "账单内容不正确";
}

// 步骤 5: 报表生成

TEST_F(AccountingSystemTest, ReportGeneration) {
    EXPECT_TRUE(account_manager->Initialize()) << "初始化失败，无法加载数据";

    std::string username = "test_user";
    std::string password = "password123";
    account_manager->RegisterUser(username, password);
    auto user = account_manager->Login(username, password);

    // 创建支出分类
    Category category1(1, "餐饮", "expense", "#FF6B6B");
    account_manager->AddCategory(*user, category1);

    // 创建收入分类
    Category category2(2, "工资", "income", "#4ECDC4");
    account_manager->AddCategory(*user, category2);

    // 创建支出账单
    Bill bill1;
    bill1.SetAmount(50.0);
    bill1.SetCategory(std::make_shared<Category>(category1));
    bill1.SetContent("午餐");
    bill1.SetTime(std::chrono::system_clock::now());
    account_manager->AddBill(user->GetUserId(), bill1);

    // 创建收入账单
    Bill bill2;
    bill2.SetAmount(5000.0);
    bill2.SetCategory(std::make_shared<Category>(category2));
    bill2.SetContent("月工资");
    bill2.SetTime(std::chrono::system_clock::now());
    account_manager->AddBill(user->GetUserId(), bill2);

    // 生成报表
    Report report = account_manager->GenerateReport(
        user->GetUserId(),
        QueryCriteria(),
        Period::kMonthly,
        ChartType::kBar
    );

    EXPECT_EQ(report.GetPeriod(), Period::kMonthly) << "报表周期不正确";
    EXPECT_EQ(report.GetChartType(), ChartType::kBar) << "报表图表类型不正确";
    EXPECT_GT(report.GetTotalIncome(), 0.0) << "报表收入总额不正确";
    EXPECT_GT(report.GetTotalExpense(), 0.0) << "报表支出总额不正确";
}
// 步骤 6: 数据持久化
TEST_F(AccountingSystemTest, DataPersistence) {
    EXPECT_TRUE(account_manager->Initialize()) << "初始化失败，无法加载数据";

    std::string username = "test_user";
    std::string password = "password123";
    account_manager->RegisterUser(username, password);
    auto user = account_manager->Login(username, password);

    // 创建分类
    Category category1(1, "餐饮", "expense", "#FF6B6B");
    account_manager->AddCategory(*user, category1);

    // 创建账单
    Bill bill;
    bill.SetAmount(50.0);
    bill.SetCategory(std::make_shared<Category>(category1));
    bill.SetContent("午餐");
    bill.SetTime(std::chrono::system_clock::now());
    account_manager->AddBill(user->GetUserId(), bill);

    // 保存数据
    EXPECT_TRUE(account_manager->SaveAll()) << "数据保存失败";

    // 重新加载数据
    auto storage2 = std::make_shared<JsonStorage>(test_data_dir);
    AccountManager account_manager2(storage2);
    EXPECT_TRUE(account_manager2.Initialize()) << "重新加载数据失败";

    // 验证数据是否一致
    auto user2 = account_manager2.Login(username, password);
    auto reloaded_bills = account_manager2.GetBills(user2->GetUserId());
    EXPECT_EQ(reloaded_bills.size(), 1) << "账单数量不一致";
    EXPECT_EQ(reloaded_bills[0].GetContent(), "午餐") << "账单内容不一致";
}

// ==================== 主程序入口 ====================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
