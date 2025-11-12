#include "models/report.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace accounting {

Report::Report()
    : period_(Period::kMonthly),
      chart_type_(ChartType::kBar),
      category_summary_(),
      total_income_(0.0),
      total_expense_(0.0) {}

Report::Report(Period period,
               ChartType chart_type,
               const std::unordered_map<std::string, double>& category_summary)
    : period_(period),
      chart_type_(chart_type),
      category_summary_(category_summary),
      total_income_(0.0),
      total_expense_(0.0) {}

// 匹配函数：判断某笔账单是否符合查询条件
bool Report::MatchCriteria(const BillData& bill, const QueryCriteria& criteria) {
    // 日期范围过滤
    auto time = bill.GetTime();
    if (criteria.HasDateRange()) {
        if (time < criteria.GetStartDate() || time > criteria.GetEndDate()) {
            return false;
        }
    }

    // 分类过滤
    if (criteria.HasCategoryFilter()) {
        if (bill.GetCategoryName() != criteria.GetCategoryName()) {
            return false;
        }
    }

    return true;
}

// 报表生成逻辑
Report Report::Generate(const std::vector<BillData>& bills,
                        const QueryCriteria& criteria,
                        Period period,
                        ChartType chart_type) {
    std::unordered_map<std::string, double> category_summary;
    double total_income = 0.0;
    double total_expense = 0.0;

    for (const auto& bill : bills) {
        if (!MatchCriteria(bill, criteria)) continue;

        double amount = bill.GetAmount();
        const std::string& category = bill.GetCategoryName();

        // 若分类为空，归为“未分类”
        std::string key = category.empty() ? "Uncategorized" : category;
        category_summary[key] += amount;

        // 根据金额正负判断收入/支出
        if (amount >= 0)
            total_income += amount;
        else
            total_expense += amount;
    }

    Report report(period, chart_type, category_summary);
    report.total_income_ = total_income;
    report.total_expense_ = total_expense;

    return report;
}

// Getters / Setters
Period Report::GetPeriod() const { return period_; }
void Report::SetPeriod(Period period) { period_ = period; }

ChartType Report::GetChartType() const { return chart_type_; }
void Report::SetChartType(ChartType chart_type) { chart_type_ = chart_type; }

const std::unordered_map<std::string, double>& Report::GetCategorySummary() const {
    return category_summary_;
}

void Report::SetCategorySummary(const std::unordered_map<std::string, double>& summary) {
    category_summary_ = summary;
}

double Report::GetTotalIncome() const { return total_income_; }
double Report::GetTotalExpense() const { return total_expense_; }

std::string Report::ToString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "Report(Period=" << static_cast<int>(period_)
        << ", ChartType=" << static_cast<int>(chart_type_)
        << ", TotalIncome=" << total_income_
        << ", TotalExpense=" << total_expense_
        << ", CategorySummary={";

    bool first = true;
    for (const auto& [cat, val] : category_summary_) {
        if (!first) oss << ", ";
        oss << cat << ": " << val;
        first = false;
    }
    oss << "})";

    return oss.str();
}

}  // namespace accounting
