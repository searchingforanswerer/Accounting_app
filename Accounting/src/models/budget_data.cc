#include "models/budget_data.h"
#include <sstream>

namespace accounting {

BudgetData::BudgetData()
        : total_limit_(0.0),
            category_limits_() {}

BudgetData::BudgetData(double total_limit, 
                                             const std::unordered_map<int, double>& category_limits)
        : total_limit_(total_limit),
            category_limits_(category_limits) {}

double BudgetData::GetTotalLimit() const { return total_limit_; }
void BudgetData::SetTotalLimit(double total_limit) { total_limit_ = total_limit; }

const std::unordered_map<int, double>& BudgetData::GetCategoryLimits() const {
    return category_limits_;
}

void BudgetData::SetCategoryLimits(const std::unordered_map<int, double>& category_limits) {
    category_limits_ = category_limits;
}

void BudgetData::AddCategoryLimit(int category_id, double limit) {
    category_limits_[category_id] = limit;
}

bool BudgetData::RemoveCategoryLimit(int category_id) {
    return category_limits_.erase(category_id) > 0;
}

double BudgetData::GetCategoryLimit(int category_id) const {
    auto it = category_limits_.find(category_id);
    if (it != category_limits_.end()) {
        return it->second;
    }
    return 0.0;
}

std::string BudgetData::ToString() const {
    std::ostringstream oss;
    oss << "BudgetData(TotalLimit: " << total_limit_ << ", CategoryLimits: {";
    
    bool first = true;
    for (const auto& [category_id, limit] : category_limits_) {
        if (!first) oss << ", ";
        oss << category_id << ": " << limit;
        first = false;
    }
    oss << "})";
    
    return oss.str();
}

}  // namespace accounting