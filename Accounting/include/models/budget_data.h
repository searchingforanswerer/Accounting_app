#ifndef ACCOUNTING_MODELS_BUDGET_DATA_H_
#define ACCOUNTING_MODELS_BUDGET_DATA_H_

#include <unordered_map>
#include <string>

namespace accounting {

class BudgetData {
public:
    // 默认构造
    BudgetData();

    // 带参构造
    BudgetData(double total_limit, 
               const std::unordered_map<int, double>& category_limits);

    // 拷贝与赋值默认即可
    BudgetData(const BudgetData&) = default;
    BudgetData& operator=(const BudgetData&) = default;

    // Getter / Setter
    double GetTotalLimit() const;
    void SetTotalLimit(double total_limit);

    const std::unordered_map<int, double>& GetCategoryLimits() const;
    void SetCategoryLimits(const std::unordered_map<int, double>& category_limits);
    
    // 便捷方法
    void AddCategoryLimit(int category_id, double limit);
    bool RemoveCategoryLimit(int category_id);
    double GetCategoryLimit(int category_id) const;

    // 调试输出
    std::string ToString() const;

private:
    double total_limit_;
    std::unordered_map<int, double> category_limits_;
};

}  // namespace accounting

#endif  // ACCOUNTING_MODELS_BUDGET_DATA_H_