#ifndef ACCOUNTING_MODELS_BILL_H_
#define ACCOUNTING_MODELS_BILL_H_

#include "models/category.h"
#include <chrono>
#include <memory>
#include <string>
#include <iostream>
#include <nlohmann/json.hpp>

namespace accounting {

using nlohmann::json;

class Bill {
public:
    // 默认构造
    Bill();

    // 带参构造
    Bill(int bill_id, double amount,
        std::shared_ptr<Category> category,
        const std::chrono::system_clock::time_point& time,
        const std::string& content);

    // 拷贝与赋值默认即可
    Bill(const Bill&) = default;
    Bill& operator=(const Bill&) = default;

    // Getter / Setter
    int GetBillId() const;
    void SetBillId(int id);

    double GetAmount() const;
    void SetAmount(double amount);

    std::shared_ptr<Category> GetCategory() const;
    void SetCategory(const std::shared_ptr<Category>& category);
    // category id helper (used during serialization / loading)
    int GetCategoryId() const;
    void SetCategoryId(int category_id);

    std::chrono::system_clock::time_point GetTime() const;
    void SetTime(const std::chrono::system_clock::time_point& time);

    const std::string& GetContent() const;
    void SetContent(const std::string& content);

    // 调试输出
    std::string ToString() const;

    // 友元 JSON 序列化函数
    friend void to_json(json& j, const Bill& b);
    friend void from_json(const json& j, Bill& b);

    private:
    int bill_id_;
    double amount_;
    std::shared_ptr<Category> category_;
    // persisted category id loaded from JSON; used to restore shared_ptr later
    int category_id_;
    std::chrono::system_clock::time_point time_;
    std::string content_;
};


}  // namespace accounting

#endif  // ACCOUNTING_MODELS_BILL_H_
