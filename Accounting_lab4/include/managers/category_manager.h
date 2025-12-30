#ifndef ACCOUNTING_MANAGER_CATEGORY_MANAGER_H_
#define ACCOUNTING_MANAGER_CATEGORY_MANAGER_H_

#include <map>
#include <vector>
#include <memory>
#include <string>
#include "models/category.h"
#include "models/user.h"

namespace accounting {

// CategoryManager 负责管理每个用户的分类集合。
class CategoryManager {
public:
    CategoryManager() = default;
    explicit CategoryManager(std::shared_ptr<class Storage> storage);

    // 添加分类（返回是否成功，若该用户已存在同名分类则失败）
    bool AddCategory(const User& user, const Category& category);

    // 更新分类信息（根据 category_id 匹配）
    bool UpdateCategory(const User& user, const Category& category);

    // 删除分类（根据 category_id 删除）
    bool DeleteCategory(const User& user, int category_id);

    // 查询接口
    std::vector<Category> GetCategoriesForUser(const User& user) const;
    const Category* GetCategoryById(const User& user, int category_id) const;
    const Category* GetCategoryByName(const User& user, const std::string& name) const;

    // 持久化接口（可选）
    bool LoadFromStorage();
    bool SaveToStorage() const;

private:
    // 检查用户的分类名是否重复
    bool IsDuplicateCategoryName(const User& user, const std::string& name) const;

    // 映射结构：一个用户对应若干分类
    std::map<int, std::vector<Category>> categories_by_user_;
    std::shared_ptr<Storage> storage_;  // 可选外部存储层
};

}  // namespace accounting

#endif  // ACCOUNTING_MANAGER_CATEGORY_MANAGER_H_
