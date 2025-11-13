#include "managers/category_manager.h"
#include "storage/storage.h"

#include <algorithm>

namespace accounting {

CategoryManager::CategoryManager(std::shared_ptr<Storage> storage)
    : storage_(std::move(storage)) {}

bool CategoryManager::AddCategory(const User& user, const Category& category) {
    auto& user_categories = categories_by_user_[user.GetUserId()];

    if (IsDuplicateCategoryName(user, category.GetName())) {
        return false;  // 名称重复
    }
    // 复制一份，可以随便改 id
    Category new_cat = category;

    // 为该用户生成一个新的 category_id
    int new_id = 1;
    for (const auto& c : user_categories) {
        if (c.GetCategoryId() >= new_id) {
            new_id = c.GetCategoryId() + 1;
        }
    }
    new_cat.SetCategoryId(new_id);


    user_categories.push_back(new_cat);
    return true;
}

bool CategoryManager::UpdateCategory(const User& user, const Category& category) {
    auto it = categories_by_user_.find(user.GetUserId());
    if (it == categories_by_user_.end()) return false;

    auto& user_categories = it->second;
    for (auto& c : user_categories) {
        if (c.GetCategoryId() == category.GetCategoryId()) {
            // 若修改名称，检查是否冲突
            if (c.GetName() != category.GetName() &&
                IsDuplicateCategoryName(user, category.GetName())) {
                return false;
            }
            c = category;
            return true;
        }
    }
    return false;
}

bool CategoryManager::DeleteCategory(const User& user, int category_id) {
    auto it = categories_by_user_.find(user.GetUserId());
    if (it == categories_by_user_.end()) return false;

    auto& user_categories = it->second;
    auto new_end = std::remove_if(user_categories.begin(), user_categories.end(),
                                  [category_id](const Category& c) {
                                      return c.GetCategoryId() == category_id;
                                  });
    if (new_end == user_categories.end()) {
        return false;  // 没找到
    }

    user_categories.erase(new_end, user_categories.end());
    return true;
}

std::vector<Category> CategoryManager::GetCategoriesForUser(const User& user) const {
    auto it = categories_by_user_.find(user.GetUserId());
    if (it == categories_by_user_.end()) return {};
    return it->second;
}

const Category* CategoryManager::GetCategoryById(const User& user, int category_id) const {
    auto it = categories_by_user_.find(user.GetUserId());
    if (it == categories_by_user_.end()) return nullptr;

    const auto& user_categories = it->second;
    for (const auto& c : user_categories) {
        if (c.GetCategoryId() == category_id) {
            return &c;
        }
    }
    return nullptr;
}

const Category* CategoryManager::GetCategoryByName(const User& user, const std::string& name) const {
    auto it = categories_by_user_.find(user.GetUserId());
    if (it == categories_by_user_.end()) return nullptr;

    const auto& user_categories = it->second;
    for (const auto& c : user_categories) {
        if (c.GetName() == name) {
            return &c;
        }
    }
    return nullptr;
}

bool CategoryManager::IsDuplicateCategoryName(const User& user, const std::string& name) const {
    auto it = categories_by_user_.find(user.GetUserId());
    if (it == categories_by_user_.end()) return false;

    const auto& user_categories = it->second;
    return std::any_of(user_categories.begin(), user_categories.end(),
                       [&name](const Category& c) { return c.GetName() == name; });
}

// 以下持久化接口暂留空实现（JSON / DB 后期可扩展）
bool CategoryManager::LoadFromStorage() {
    if (!storage_) return false;
    categories_by_user_.clear();
    try {
        auto res = storage_->LoadCategoriesByUser();
        if (!res.first) return false;
        categories_by_user_ = std::move(res.second);
    } catch (...) {
        return false;
    }
    return true;
}

bool CategoryManager::SaveToStorage() const {
    if (!storage_) return false;
    return storage_->SaveCategoriesByUser(categories_by_user_);
}

}  // namespace accounting
