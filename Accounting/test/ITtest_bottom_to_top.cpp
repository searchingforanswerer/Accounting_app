#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <memory>
#include <vector>
#include "core/account_manager.h"
#include "managers/category_manager.h"
#include "models/bill.h"
#include "models/category.h"
#include "models/user.h"
#include "storage/storage.h"
#include "storage/json_storage.h"

using namespace accounting;

// ==================== JsonStorage 测试 ====================
class JsonStorageTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_data_dir = "./test_data/test_json_storage";
        std::filesystem::create_directory(test_data_dir);  // 创建测试数据目录
        storage = std::make_shared<JsonStorage>(test_data_dir);  // 使用 JsonStorage 存储
    }

    void TearDown() override {
        // 清理工作
        std::filesystem::remove_all(test_data_dir);  // 删除测试数据目录
    }

    std::shared_ptr<JsonStorage> storage;
    std::string test_data_dir;
};

// 测试：保存并加载分类
TEST_F(JsonStorageTest, TestSaveLoadCategories) {
    std::vector<Category> categories = {
        Category(1, "餐饮", "expense", "#FF6B6B"),
        Category(2, "交通", "expense", "#4ECDC4")
    };

    // 测试保存分类数据
    std::map<int, std::vector<Category>> categories_by_user;
    categories_by_user[1] = categories;
    ASSERT_TRUE(storage->SaveCategoriesByUser(categories_by_user)) << "保存分类数据失败";

    // 测试加载分类数据
    auto result = storage->LoadCategoriesByUser();
    ASSERT_TRUE(result.first) << "加载分类数据失败";
    auto loaded_categories = result.second[1];
    ASSERT_EQ(loaded_categories.size(), 2) << "加载的分类数量不对";
    ASSERT_EQ(loaded_categories[0].GetName(), "餐饮") << "分类名称不对";
}

// ==================== CategoryManager 测试 ====================
class CategoryManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        data_dir = "./test_data/test_category_manager";
        std::filesystem::create_directory(data_dir);  // 创建独立目录

        auto storage = std::make_shared<JsonStorage>(data_dir);
        category_manager = std::make_shared<CategoryManager>(storage);
        category_manager->LoadFromStorage();  // 加载存储数据
    }

    void TearDown() override {
        category_manager.reset();
        std::filesystem::remove_all(data_dir);  // 清除临时数据
    }

    std::shared_ptr<CategoryManager> category_manager;
    std::string data_dir;
};

// 测试：通过 CategoryManager 添加分类
TEST_F(CategoryManagerTest, TestAddCategory) {
    User user(1, "test_user");
    Category category(1, "餐饮", "expense", "#FF6B6B");

    ASSERT_TRUE(category_manager->AddCategory(user, category)) << "添加分类失败";

    auto categories = category_manager->GetCategoriesForUser(user);
    ASSERT_EQ(categories.size(), 1) << "分类数量不对";
    ASSERT_EQ(categories[0].GetName(), "餐饮") << "分类名称不对";
}

// ==================== AccountManager 测试 ====================
class AccountManagerTest : public ::testing::Test {
protected:
    AccountManagerTest() {
        data_dir = "./test_data/test_account_manager";
        std::filesystem::create_directory(data_dir);  // 每个测试用例独立目录
    }

    User user{1, "test_user"};
    std::shared_ptr<CategoryManager> category_manager;
    std::shared_ptr<AccountManager> account_manager;
    std::shared_ptr<Storage> storage;
    std::string data_dir;

    void SetUp() override {
        std::filesystem::remove_all(data_dir);
        std::filesystem::create_directory(data_dir);

        storage = std::make_shared<JsonStorage>(data_dir);
        account_manager = std::make_shared<AccountManager>(storage);
        category_manager = std::make_shared<CategoryManager>(storage);
        category_manager->LoadFromStorage();

        Category food_category;
        food_category.SetCategoryId(1);
        food_category.SetName("Food");
        category_manager->AddCategory(user, food_category);

        Category transport_category;
        transport_category.SetCategoryId(2);
        transport_category.SetName("Transport");
        category_manager->AddCategory(user, transport_category);

        account_manager->Initialize();
    }

    void TearDown() override {
        account_manager.reset();
        category_manager.reset();
        std::filesystem::remove_all(data_dir);
    }
};

// 测试：用户分类集成（注册、登录、分类、账单）
TEST_F(AccountManagerTest, TestUserCategoryIntegration) {
    std::string username = "test_user";
    std::string password = "password123";

    // 注册并登录用户
    ASSERT_TRUE(account_manager->RegisterUser(username, password));
    auto user_ptr = account_manager->Login(username, password);
    ASSERT_TRUE(user_ptr != nullptr) << "用户登录失败";

    // 添加分类
    Category category(1, "餐饮", "expense", "#FF6B6B");
    ASSERT_TRUE(account_manager->AddCategory(*user_ptr, category)) << "添加分类失败";

    auto categories = account_manager->GetCategories(*user_ptr);
    ASSERT_EQ(categories.size(), 1) << "分类数量不对";
    ASSERT_EQ(categories[0].GetName(), "餐饮") << "分类名称不对";

    // 添加账单
    Bill bill;
    bill.SetAmount(50.0);
    bill.SetCategory(std::make_shared<Category>(categories[0]));
    bill.SetContent("午餐");
    bill.SetTime(std::chrono::system_clock::now());

    ASSERT_TRUE(account_manager->AddBill(user_ptr->GetUserId(), bill)) << "账单添加失败";

    auto bills = account_manager->GetBills(user_ptr->GetUserId());
    ASSERT_EQ(bills.size(), 1) << "账单数量不对";
    ASSERT_EQ(bills[0].GetContent(), "午餐") << "账单内容不对";
}

// ==================== Main 函数 ====================
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
