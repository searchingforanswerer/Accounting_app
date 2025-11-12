#include "managers/report_manager.h"
#include <stdexcept>

namespace accounting {

ReportManager::ReportManager(BillManager* bill_manager)
    : bill_manager_(bill_manager) {
    if (!bill_manager_) {
        throw std::invalid_argument("BillManager pointer cannot be null.");
    }
}

Report ReportManager::GenerateReport(int user_id,
                                     const QueryCriteria& criteria,
                                     Period period,
                                     ChartType chart_type) {
    // 从 BillManager 获取用户账单
    std::vector<Bill> bills = bill_manager_->GetBillsByUser(user_id);

    // 将 Bill 转为 BillData
    std::vector<BillData> bill_data_list;
    for (const auto& bill : bills) {
        std::string category_name = bill.GetCategory() ? bill.GetCategory()->GetName() : "";
        bill_data_list.emplace_back(bill.GetAmount(),
                                    category_name,
                                    bill.GetTime(),
                                    bill.GetContent());
    }

    // 生成报表
    Report report = Report::Generate(bill_data_list, criteria, period, chart_type);

    // 缓存到 reports_
    reports_[user_id].push_back(report);

    return report;
}

std::optional<Report> ReportManager::GetLastReport(int user_id) const {
    auto it = reports_.find(user_id);
    if (it == reports_.end() || it->second.empty()) {
        return std::nullopt;
    }
    return it->second.back();
}

const std::vector<Report>& ReportManager::GetReportsByUser(int user_id) const {
    static const std::vector<Report> kEmpty;  // 用于返回空引用
    auto it = reports_.find(user_id);
    return (it != reports_.end()) ? it->second : kEmpty;
}

void ReportManager::ClearReports(int user_id) {
    reports_.erase(user_id);
}

}  // namespace accounting
