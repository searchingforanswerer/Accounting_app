#ifndef ACCOUNTING_MODELS_REPORT_H_
#define ACCOUNTING_MODELS_REPORT_H_

#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>

#include "models/period.h"
#include "models/chart_type.h"
#include "models/bill_data.h"
#include "models/query_criteria.h"

namespace accounting {

class Report {
public:
    // 默认构造
    Report();

    // 带参构造
    Report(Period period,
           ChartType chart_type,
           const std::unordered_map<std::string, double>& category_summary);

    // 生成报表的静态工厂函数
    static Report Generate(const std::vector<BillData>& bills,
                           const QueryCriteria& criteria,
                           Period period,
                           ChartType chart_type);

    // Getter / Setter
    Period GetPeriod() const;
    void SetPeriod(Period period);

    ChartType GetChartType() const;
    void SetChartType(ChartType chart_type);

    const std::unordered_map<std::string, double>& GetCategorySummary() const;
    void SetCategorySummary(const std::unordered_map<std::string, double>& summary);

    double GetTotalIncome() const;
    double GetTotalExpense() const;

    std::string ToString() const;

private:
    // 私有辅助方法
    static bool MatchCriteria(const BillData& bill, const QueryCriteria& criteria);

private:
    Period period_;
    ChartType chart_type_;
    std::unordered_map<std::string, double> category_summary_;  // category → total amount
    double total_income_;
    double total_expense_;
};

}  // namespace accounting

#endif  // ACCOUNTING_MODELS_REPORT_H_
