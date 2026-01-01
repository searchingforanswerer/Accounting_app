#include <gtest/gtest.h>
#include "core/account_manager.h"
#include "managers/category_manager.h"
#include "models/bill.h"
#include "models/category.h"
#include "models/user.h"
#include "storage/storage.h"
#include "storage/json_storage.h"
#include <chrono>
#include <vector>

using namespace accounting;

// 测试类
class AccountManagerTest : public ::testing::Test {
protected:
    AccountManagerTest() {
        // 为每个测试用例创建独立目录
        data_dir = "./test_data/test_account_manager";  // 每个测试用例独立目录
        std::filesystem::create_directory(data_dir);  // 创建目录
    }

    // 准备一个用户对象和分类对象
    User user{1, "test_user"};
    std::shared_ptr<CategoryManager> category_manager;
    std::shared_ptr<AccountManager> account_manager;
    std::shared_ptr<Storage> storage;
    std::string data_dir;  // 存储路径

    void SetUp() override {
        // 清理上次的测试数据
        std::filesystem::remove_all(data_dir);  // 删除上次的数据
        std::filesystem::create_directory(data_dir);  // 重新创建目录
        
        // 创建存储实例，并加载数据
        storage = std::make_shared<JsonStorage>(data_dir);  // 使用 test_data 路径
        account_manager = std::make_shared<AccountManager>(storage);
        
        // 加载存储数据
        category_manager = std::make_shared<CategoryManager>(storage);
        category_manager->LoadFromStorage();
        
        // 为用户添加分类
        Category food_category;
        food_category.SetCategoryId(1);
        food_category.SetName("Food");
        category_manager->AddCategory(user, food_category);

        Category transport_category;
        transport_category.SetCategoryId(2);
        transport_category.SetName("Transport");
        category_manager->AddCategory(user, transport_category);

        // 初始化 AccountManager
        account_manager->Initialize();
    }
    
    // 清理函数
    void TearDown() override {
        // 清理工作
        account_manager.reset();
        category_manager.reset();
        
        // 删除临时存储目录
        std::filesystem::remove_all(data_dir);
    }
};

// 测试用例 1: 测试添加单个账单成功的情况
TEST_F(AccountManagerTest, TestAddSingleBill) {
    // 创建账单
    Bill bill;
    bill.SetAmount(50.0);
    bill.SetCategory(std::make_shared<Category>(category_manager->GetCategoriesForUser(user)[0])); // 假设已有“Food”类别
    bill.SetContent("Lunch");
    bill.SetTime(std::chrono::system_clock::now());

    bool result = account_manager->AddBill(user.GetUserId(), bill);
    
    ASSERT_TRUE(result) << "账单添加失败";
    std::vector<Bill> bills = account_manager->GetBills(user.GetUserId());
    ASSERT_EQ(bills.size(), 1) << "账单数量不匹配";
    ASSERT_EQ(bills[0].GetAmount(), 50.0) << "账单金额不匹配";
    ASSERT_EQ(bills[0].GetContent(), "Lunch") << "账单内容不匹配";
}

// 测试用例 2: 添加多个账单
TEST_F(AccountManagerTest, TestAddMultipleBills) {
    // 创建多个账单
    Bill bill1, bill2, bill3;
    bill1.SetAmount(100.0);
    bill1.SetCategory(std::make_shared<Category>(category_manager->GetCategoriesForUser(user)[0])); // "Food"
    bill1.SetContent("Lunch");
    bill1.SetTime(std::chrono::system_clock::now());

    bill2.SetAmount(50.0);
    bill2.SetCategory(std::make_shared<Category>(category_manager->GetCategoriesForUser(user)[1])); // "Transport"
    bill2.SetContent("Bus");
    bill2.SetTime(std::chrono::system_clock::now());

    bill3.SetAmount(200.0);
    bill3.SetCategory(std::make_shared<Category>(category_manager->GetCategoriesForUser(user)[0])); // "Food"
    bill3.SetContent("Dinner");
    bill3.SetTime(std::chrono::system_clock::now());

    account_manager->AddBill(user.GetUserId(), bill1);
    account_manager->AddBill(user.GetUserId(), bill2);
    account_manager->AddBill(user.GetUserId(), bill3);

    // 查询账单并验证
    std::vector<Bill> all_bills = account_manager->GetBills(user.GetUserId());
    ASSERT_EQ(all_bills.size(), 3) << "添加多个账单后，账单数量不匹配";
}

// 测试用例 3: 测试添加超预算账单的失败情况
TEST_F(AccountManagerTest, TestAddBillOverBudgetFailure) {
    // 设置预算
    Budget budget;
    budget.SetTotalLimit(100.0);
    budget.SetCategoryLimit(1, 50.0);  // 设置 "Food" 分类的预算为 50 元
    account_manager->SetBudget(user.GetUserId(), budget);

    // 创建超预算的账单
    Bill bill;
    bill.SetAmount(60.0);  // 超过分类预算
    bill.SetCategory(std::make_shared<Category>(category_manager->GetCategoriesForUser(user)[0]));  // "Food"
    bill.SetContent("Dinner");
    bill.SetTime(std::chrono::system_clock::now());

    bool result = account_manager->AddBill(user.GetUserId(), bill);
    
    ASSERT_FALSE(result) << "账单不应该被添加（预算超支）";
}

// 测试用例 4: 多个账单中，有超预算和未超预算的情况
TEST_F(AccountManagerTest, TestAddMultiBillsWithBudgetCheck) {
    // 设置预算
    Budget budget;
    budget.SetTotalLimit(500.0);
    budget.SetCategoryLimit(1, 200.0);  // 设置 "Food" 分类的预算为 200 元
    account_manager->SetBudget(user.GetUserId(), budget);

    // 创建账单，其中 bill1 不超预算，bill2 超预算
    Bill bill1, bill2;
    bill1.SetAmount(150.0);
    bill1.SetCategory(std::make_shared<Category>(category_manager->GetCategoriesForUser(user)[0])); // "Food"
    bill1.SetContent("Lunch");
    bill1.SetTime(std::chrono::system_clock::now());

    bill2.SetAmount(250.0);  // 超预算
    bill2.SetCategory(std::make_shared<Category>(category_manager->GetCategoriesForUser(user)[0])); // "Food"
    bill2.SetContent("Dinner");
    bill2.SetTime(std::chrono::system_clock::now());

    // 添加账单并验证
    bool result1 = account_manager->AddBill(user.GetUserId(), bill1);
    bool result2 = account_manager->AddBill(user.GetUserId(), bill2);

    ASSERT_TRUE(result1) << "账单 1 添加失败";
    ASSERT_FALSE(result2) << "账单 2 应该被预算检查阻止";
    
    // 查询账单
    std::vector<Bill> all_bills = account_manager->GetBills(user.GetUserId());
    ASSERT_EQ(all_bills.size(), 1) << "账单数量不匹配";
    ASSERT_EQ(all_bills[0].GetAmount(), 150.0) << "未超预算账单添加失败";
}

// 测试用例 5: 测试查询账单
TEST_F(AccountManagerTest, TestQueryBills) {
    // 添加账单
    Bill bill1;
    bill1.SetAmount(100.0);
    bill1.SetCategory(std::make_shared<Category>(category_manager->GetCategoriesForUser(user)[0])); // "Food"
    bill1.SetContent("Grocery");
    bill1.SetTime(std::chrono::system_clock::now());

    Bill bill2;
    bill2.SetAmount(20.0);
    bill2.SetCategory(std::make_shared<Category>(category_manager->GetCategoriesForUser(user)[1])); // "Transport"
    bill2.SetContent("Bus Ticket");
    bill2.SetTime(std::chrono::system_clock::now());

    account_manager->AddBill(user.GetUserId(), bill1);
    account_manager->AddBill(user.GetUserId(), bill2);

    // 查询账单
    std::vector<Bill> all_bills = account_manager->GetBills(user.GetUserId());
    
    ASSERT_EQ(all_bills.size(), 2) << "查询到的账单数量不匹配";
    ASSERT_EQ(all_bills[0].GetAmount(), 100.0) << "第一个账单金额不匹配";
    ASSERT_EQ(all_bills[1].GetAmount(), 20.0) << "第二个账单金额不匹配";
}

// 测试用例 6: 测试删除账单
TEST_F(AccountManagerTest, TestDeleteSingleBill) {
    // 添加账单
    Bill bill1;
    bill1.SetAmount(100.0);
    bill1.SetCategory(std::make_shared<Category>(category_manager->GetCategoriesForUser(user)[0])); // "Food"
    bill1.SetContent("Grocery");
    bill1.SetTime(std::chrono::system_clock::now());

    account_manager->AddBill(user.GetUserId(), bill1);
    
    // 获取实际的账单 ID
    std::vector<Bill> bills = account_manager->GetBills(user.GetUserId());
    ASSERT_EQ(bills.size(), 1) << "账单添加失败";
    int bill_id = bills[0].GetBillId();
    
    // 删除账单
    bool delete_result = account_manager->DeleteBill(user.GetUserId(), bill_id);
    ASSERT_TRUE(delete_result) << "账单删除失败";

    // 查询账单，确保没有账单了
    std::vector<Bill> all_bills = account_manager->GetBills(user.GetUserId());
    ASSERT_EQ(all_bills.size(), 0) << "账单删除后依然存在账单";
}

// 测试用例 7: 测试删除多个账单，确保删除逻辑正确
TEST_F(AccountManagerTest, TestDeleteMultiBill) {
    // 添加三个账单
    Bill bill1, bill2, bill3;
    
    bill1.SetAmount(100.0);
    bill1.SetCategory(std::make_shared<Category>(category_manager->GetCategoriesForUser(user)[0])); // "Food"
    bill1.SetContent("Grocery");
    bill1.SetTime(std::chrono::system_clock::now());

    bill2.SetAmount(50.0);
    bill2.SetCategory(std::make_shared<Category>(category_manager->GetCategoriesForUser(user)[1])); // "Transport"
    bill2.SetContent("Bus Ticket");
    bill2.SetTime(std::chrono::system_clock::now());

    bill3.SetAmount(200.0);
    bill3.SetCategory(std::make_shared<Category>(category_manager->GetCategoriesForUser(user)[0])); // "Food"
    bill3.SetContent("Dinner");
    bill3.SetTime(std::chrono::system_clock::now());

    account_manager->AddBill(user.GetUserId(), bill1);
    account_manager->AddBill(user.GetUserId(), bill2);
    account_manager->AddBill(user.GetUserId(), bill3);

    // 查询账单并获取实际的账单 ID
    std::vector<Bill> all_bills = account_manager->GetBills(user.GetUserId());
    ASSERT_EQ(all_bills.size(), 3) << "添加账单后，账单数量不匹配";
    
    // 提取账单 ID（按添加顺序）
    int bill1_id = all_bills[0].GetBillId();
    int bill2_id = all_bills[1].GetBillId();
    int bill3_id = all_bills[2].GetBillId();

    // 删除两个账单（删除 bill1 和 bill2）
    bool delete_result1 = account_manager->DeleteBill(user.GetUserId(), bill1_id);
    bool delete_result2 = account_manager->DeleteBill(user.GetUserId(), bill2_id);

    // 验证删除结果
    ASSERT_TRUE(delete_result1) << "账单 1 删除失败";
    ASSERT_TRUE(delete_result2) << "账单 2 删除失败";

    // 再次查询账单，确保只剩下 bill3
    all_bills = account_manager->GetBills(user.GetUserId());
    ASSERT_EQ(all_bills.size(), 1) << "删除账单后，账单数量不匹配";
    ASSERT_EQ(all_bills[0].GetBillId(), bill3_id) << "剩余账单不符合预期";
}

// 测试用例 8: 测试更新账单
TEST_F(AccountManagerTest, TestUpdateBill) {
    // 添加账单
    Bill bill1;
    bill1.SetAmount(50.0);
    bill1.SetCategory(std::make_shared<Category>(category_manager->GetCategoriesForUser(user)[0])); // "Food"
    bill1.SetContent("Grocery");
    bill1.SetTime(std::chrono::system_clock::now());
    account_manager->AddBill(user.GetUserId(), bill1);
    
    // 获取实际的账单 ID
    std::vector<Bill> bills = account_manager->GetBills(user.GetUserId());
    ASSERT_EQ(bills.size(), 1) << "账单添加失败";
    int bill_id = bills[0].GetBillId();
    
    // 更新账单
    Bill updated_bill = bill1;
    updated_bill.SetBillId(bill_id);
    updated_bill.SetAmount(60.0);
    updated_bill.SetContent("Updated Grocery");

    bool update_result = account_manager->UpdateBill(user.GetUserId(), updated_bill);
    ASSERT_TRUE(update_result) << "账单更新失败";

    // 查询账单，确保更新生效
    std::vector<Bill> all_bills = account_manager->GetBills(user.GetUserId());
    ASSERT_EQ(all_bills[0].GetAmount(), 60.0) << "更新后的账单金额不匹配";
    ASSERT_EQ(all_bills[0].GetContent(), "Updated Grocery") << "更新后的账单内容不匹配";
}

// 测试用例 9: 测试添加相同 BillId 的两个账单，应该返回 false
TEST_F(AccountManagerTest, TestAddDuplicateBillId) {
    // 创建账单
    Bill bill1, bill2;
    bill1.SetBillId(1);  // 设置相同的 BillId
    bill1.SetAmount(100.0);
    bill1.SetCategory(std::make_shared<Category>(category_manager->GetCategoriesForUser(user)[0])); // "Food"
    bill1.SetContent("Lunch");
    bill1.SetTime(std::chrono::system_clock::now());

    bill2.SetBillId(1);  // 使用相同的 BillId
    bill2.SetAmount(50.0);
    bill2.SetCategory(std::make_shared<Category>(category_manager->GetCategoriesForUser(user)[1])); // "Transport"
    bill2.SetContent("Bus");
    bill2.SetTime(std::chrono::system_clock::now());

    // 添加账单
    bool result1 = account_manager->AddBill(user.GetUserId(), bill1);
    bool result2 = account_manager->AddBill(user.GetUserId(), bill2);

    ASSERT_TRUE(result1) << "账单 1 添加失败";
    ASSERT_FALSE(result2) << "账单 2 应该由于重复的 BillId 被拒绝";
}

// 测试用例 10: 综合测试：测试添加、删除和查询的一套完整流程
TEST_F(AccountManagerTest, TestAddDeleteQueryBills) {
    // 添加账单
    Bill bill1;
    bill1.SetAmount(100.0);
    bill1.SetCategory(std::make_shared<Category>(category_manager->GetCategoriesForUser(user)[0])); // "Food"
    bill1.SetContent("Lunch");
    bill1.SetTime(std::chrono::system_clock::now());

    bool add_result = account_manager->AddBill(user.GetUserId(), bill1);
    ASSERT_TRUE(add_result) << "账单添加失败";

    // 查询账单
    std::vector<Bill> all_bills = account_manager->GetBills(user.GetUserId());
    ASSERT_EQ(all_bills.size(), 1) << "查询账单数量不匹配";

    // 获取实际的账单 ID
    int bill_id = all_bills[0].GetBillId();
    
    // 删除账单
    bool delete_result = account_manager->DeleteBill(user.GetUserId(), bill_id);
    ASSERT_TRUE(delete_result) << "账单删除失败";

    // 再次查询账单，确保账单已被删除
    all_bills = account_manager->GetBills(user.GetUserId());
    ASSERT_EQ(all_bills.size(), 0) << "删除账单后，账单数量不匹配";
}