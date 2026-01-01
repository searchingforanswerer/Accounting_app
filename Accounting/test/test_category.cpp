#include <gtest/gtest.h>
#include <fstream>
#include "managers/category_manager.h"
#include "core/account_manager.h"
#include "storage/json_storage.h"
#include "models/category.h"
#include "models/user.h"
#include <memory>
#include <vector>
#include <filesystem>

using namespace accounting;

// 测试类：CategoryManager 持久化功能
class CategoryManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 假设测试使用一个文件存储作为后端
        data_dir = "./test_data/test_category";  // 你可以选择其他路径
        std::filesystem::create_directory(data_dir);
        
        // 需要清空上次的测试数据
        std::filesystem::remove_all(data_dir);
        std::filesystem::create_directory(data_dir);

        auto storage = std::make_shared<JsonStorage>(data_dir);
        category_manager = std::make_shared<CategoryManager>(storage);

        // 测试前先加载存储
        category_manager->LoadFromStorage();
    }

    void TearDown() override {
        // 清理工作
        category_manager.reset();
        // 清空文件夹，删除临时文件
        std::filesystem::remove_all(data_dir);
    }

    std::shared_ptr<CategoryManager> category_manager;
    std::string data_dir;
};

class CategoryManagerFailureTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时文件夹用于存储
        data_dir = "./test_data/test_category";
        std::filesystem::create_directory(data_dir);
        
        // 创建存储实例并加载 CategoryManager
        auto storage = std::make_shared<JsonStorage>(data_dir);
        category_manager = std::make_shared<CategoryManager>(storage);
        
        // 清空数据并加载初始数据
        category_manager->LoadFromStorage();
    }

    void TearDown() override {
        // 清理
        category_manager.reset();
        std::filesystem::remove_all(data_dir);
    }

    std::shared_ptr<CategoryManager> category_manager;
    std::string data_dir;
};


// 测试用例 1: 测试类别添加功能
TEST_F(CategoryManagerTest, TestAddCategory) {
    User user(1, "test_user");
    Category category(1, "餐饮", "expense", "#FF6B6B");
    
    // 添加分类
    ASSERT_TRUE(category_manager->AddCategory(user, category));

    // 获取并验证添加后的分类
    auto categories = category_manager->GetCategoriesForUser(user);
    ASSERT_EQ(categories.size(), 1);
    EXPECT_EQ(categories[0].GetName(), "餐饮");
}

// 测试用例 2: 尝试添加同名分类，失败用例
TEST_F(CategoryManagerFailureTest, TestAddDuplicateCategoryName) {
    User user(1, "test_user");

    // 添加一个分类
    Category category(1, "餐饮", "expense", "#FF6B6B");
    ASSERT_TRUE(category_manager->AddCategory(user, category));

    // 尝试添加一个同名分类
    Category duplicate_category(2, "餐饮", "expense", "#00FF00");
    ASSERT_FALSE(category_manager->AddCategory(user, duplicate_category));  // 失败：名称重复
}

// 测试用例 3: 测试类别更新功能
TEST_F(CategoryManagerTest, TestUpdateCategory) {
    User user(1, "test_user");
    Category category(1, "餐饮", "expense", "#FF6B6B");
    
    // 添加分类
    category_manager->AddCategory(user, category);
    
    // 更新分类
    Category updated_category(1, "餐饮", "expense", "#00FF00");
    ASSERT_TRUE(category_manager->UpdateCategory(user, updated_category));
    
    // 获取并验证更新后的分类
    auto categories = category_manager->GetCategoriesForUser(user);
    ASSERT_EQ(categories.size(), 1);
    EXPECT_EQ(categories[0].GetName(), "餐饮");
    EXPECT_EQ(categories[0].GetColor(), "#00FF00");
}

// 测试用例 4: 尝试更新不存在的分类，失败用例
TEST_F(CategoryManagerFailureTest, TestUpdateNonExistentCategory) {
    User user(1, "test_user");

    // 尝试更新一个不存在的分类
    Category non_existent_category(99, "未知", "expense", "#0000FF");
    ASSERT_FALSE(category_manager->UpdateCategory(user, non_existent_category));  // 失败：分类不存在
}

// 测试用例 5: 尝试更新分类名称冲突，失败用例
TEST_F(CategoryManagerFailureTest, TestUpdateCategoryWithDuplicateName) {
    User user(1, "test_user");

    // 添加一个分类
    Category category1(1, "餐饮", "expense", "#FF6B6B");
    category_manager->AddCategory(user, category1);
    
    // 添加另一个不同名称的分类
    Category category2(2, "娱乐", "income", "#00FF00");
    category_manager->AddCategory(user, category2);

    // 尝试更新分类1的名称为分类2的名称（重复）
    Category updated_category(1, "娱乐", "expense", "#FF6B6B");
    ASSERT_FALSE(category_manager->UpdateCategory(user, updated_category));  // 失败：名称冲突
}

// 测试用例 6: 测试类别删除功能
TEST_F(CategoryManagerTest, TestDeleteCategory) {
    User user(1, "test_user");
    Category category(1, "餐饮", "expense", "#FF6B6B");

    // 添加分类
    category_manager->AddCategory(user, category);

    // 删除分类
    ASSERT_TRUE(category_manager->DeleteCategory(user, 1));
    
    // 获取并验证删除后的分类
    auto categories = category_manager->GetCategoriesForUser(user);
    ASSERT_EQ(categories.size(), 0);
}

// 测试用例 7: 尝试删除不存在的分类，失败用例
TEST_F(CategoryManagerFailureTest, TestDeleteNonExistentCategory) {
    User user(1, "test_user");

    // 尝试删除一个不存在的分类
    ASSERT_FALSE(category_manager->DeleteCategory(user, 99));  // 失败：分类不存在
}

// 测试用例 8: 测试重名检测
TEST_F(CategoryManagerTest, TestIsDuplicateCategoryName) {
    User user(1, "test_user");
    Category cat1(0, "餐饮", "expense", "#FF6B6B");
    Category cat2(0, "餐饮", "expense", "#FF6B6B");

    // 添加分类
    category_manager->AddCategory(user, cat1);

    bool result = category_manager->TestIsDuplicateCategoryNameForTest(user, "餐饮");  // 检查重复名称
    EXPECT_TRUE(result);  // 期望名称重复
}

// 测试用例 9: 尝试获取用户没有分类的情况
TEST_F(CategoryManagerFailureTest, TestGetCategoriesForUserWithNoCategories) {
    User user(2, "new_user");

    // 获取没有分类的用户的分类
    auto categories = category_manager->GetCategoriesForUser(user);
    ASSERT_TRUE(categories.empty());  // 应该返回空的分类列表
}

// 测试用例 10: 测试数据加载功能
TEST_F(CategoryManagerTest, TestLoadFromStorage) {
    User user(1, "test_user");
    Category category(1, "餐饮", "expense", "#FF6B6B");
    
    // 添加分类并保存
    category_manager->AddCategory(user, category);
    category_manager->SaveToStorage();
    
    // 创建新的 CategoryManager 来模拟重新加载数据
    auto new_storage = std::make_shared<JsonStorage>(data_dir);
    CategoryManager new_category_manager(new_storage);
    ASSERT_TRUE(new_category_manager.LoadFromStorage());

    // 获取并验证重新加载的分类
    auto categories = new_category_manager.GetCategoriesForUser(user);
    ASSERT_EQ(categories.size(), 1);
    EXPECT_EQ(categories[0].GetName(), "餐饮");
}

// 测试用例 11: 测试数据保存功能
TEST_F(CategoryManagerTest, TestSaveToStorage) {
    User user(1, "test_user");
    Category category(1, "餐饮", "expense", "#FF6B6B");
    
    // 添加分类并保存
    category_manager->AddCategory(user, category);
    ASSERT_TRUE(category_manager->SaveToStorage());

    // 检查文件是否创建并包含数据
    std::ifstream ifs(data_dir + "/categories.json");
    ASSERT_TRUE(ifs.is_open());
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("餐饮"), std::string::npos);  // 确保文件中包含"餐饮"这个关键词
}
