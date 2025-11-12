#ifndef ACCOUNTING_MANAGERS_REPORT_MANAGER_H_
#define ACCOUNTING_MANAGERS_REPORT_MANAGER_H_

#include <unordered_map>
#include <vector>
#include <string>
#include <optional>

#include "models/report.h"
#include "models/query_criteria.h"
#include "models/user.h"
#include "managers/bill_manager.h"

namespace accounting {

class ReportManager {
public:
    explicit ReportManager(BillManager* bill_manager);

    // 为指定用户生成报表
    Report GenerateReport(int user_id,
                          const QueryCriteria& criteria,
                          Period period,
                          ChartType chart_type);

    // 获取最近一次生成的报表（若存在）
    std::optional<Report> GetLastReport(int user_id) const;

    // 获取某用户的所有历史报表
    const std::vector<Report>& GetReportsByUser(int user_id) const;

    // 清空某用户的报表缓存
    void ClearReports(int user_id);

private:
    BillManager* bill_manager_;  // 指向账单管理器，解耦依赖
    std::unordered_map<int, std::vector<Report>> reports_; // user_id -> reports
};

}  // namespace accounting

#endif  // ACCOUNTING_MANAGERS_REPORT_MANAGER_H_
