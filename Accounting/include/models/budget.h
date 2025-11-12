#ifndef ACCOUNTING_MODELS_BUDGET_H_
#define ACCOUNTING_MODELS_BUDGET_H_

#include "models/category.h"
#include <unordered_map>
#include <memory>
#include <nlohmann/json.hpp>

using nlohmann::json;

namespace accounting {

class Budget {
public:
    // 默认构造
    Budget();

    // 带参构造
    Budget(double total_limit, const std::unordered_map<int, double>& category_limits);

    // 拷贝与赋值默认即可
    Budget(const Budget&) = default;
    Budget& operator=(const Budget&) = default;

    // Getter / Setter
    double GetTotalLimit() const;
    void SetTotalLimit(double limit);

    const std::unordered_map<int, double>& GetCategoryLimits() const;
    void SetCategoryLimit(int category_id, double limit);
    double GetCategoryLimit(int category_id) const;

    // 调试输出
    std::string ToString() const;

    // JSON 序列化支持
    friend void to_json(json& j, const Budget& b);
    friend void from_json(const json& j, Budget& b);

private:
    double total_limit_;
    std::unordered_map<int, double> category_limits_;
};

}  // namespace accounting

#endif  // ACCOUNTING_MODELS_BUDGET_H_
