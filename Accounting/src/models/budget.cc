#include "models/budget.h"
#include <sstream>
#include <iostream>

namespace accounting {

// 默认构造
Budget::Budget() : total_limit_(0.0) {}

// 带参构造
Budget::Budget(double total_limit, const std::unordered_map<int, double>& category_limits)
    : total_limit_(total_limit), category_limits_(category_limits) {}

// Getter / Setter
double Budget::GetTotalLimit() const { return total_limit_; }
void Budget::SetTotalLimit(double limit) { total_limit_ = limit; }

const std::unordered_map<int, double>& Budget::GetCategoryLimits() const {
    return category_limits_;
}

void Budget::SetCategoryLimit(int category_id, double limit) {
    // negative id treated as invalid
    if (category_id >= 0) {
        category_limits_[category_id] = limit;
    }
}

double Budget::GetCategoryLimit(int category_id) const {
    auto it = category_limits_.find(category_id);
    if (it != category_limits_.end()) {
        return it->second;
    }
    return 0.0;
}

// 调试输出
std::string Budget::ToString() const {
    std::ostringstream oss;
    oss << "Budget(TotalLimit: " << total_limit_ << ", CategoryLimits: {";
    bool first = true;
    for (const auto& [category_id, limit] : category_limits_) {
        if (!first) oss << ", ";
        oss << category_id << ": " << limit;
        first = false;
    }
    oss << "})";
    return oss.str();
}
 // namespace accounting

// JSON 序列化实现

void to_json(json& j, const Budget& b) {
    // 推荐使用数组格式以避免 JSON 对象键必须为字符串的问题
    // {
    //   "total_limit": 100.0,
    //   "category_limits": [ {"category_id": 1, "limit": 50.0}, ... ]
    // }
    j["total_limit"] = b.total_limit_;
    j["category_limits"] = json::array();
    for (const auto& [category_id, limit] : b.category_limits_) {
        j["category_limits"].push_back(json{{"category_id", category_id}, {"limit", limit}});
    }
}

void from_json(const json& j, Budget& b) {
    b.total_limit_ = j.value("total_limit", 0.0);
    b.category_limits_.clear();

    if (j.contains("category_limits")) {
        const json& arr = j.at("category_limits");
        if (arr.is_array()) {
            for (const auto& item : arr) {
                int cid = item.value("category_id", -1);
                double limit = item.value("limit", 0.0);
                if (cid >= 0) b.category_limits_[cid] = limit;
            }
        } else if (arr.is_object()) {
            // 向后兼容：旧版可能使用对象并把 key 作为字符串
            for (auto it = arr.begin(); it != arr.end(); ++it) {
                // key is string, try parse as int
                try {
                    int cid = std::stoi(it.key());
                    double limit = it.value().get<double>();
                    if (cid >= 0) b.category_limits_[cid] = limit;
                } catch (...) {
                    // ignore non-int keys
                }
            }
        }
    }
}

}  // namespace accounting
