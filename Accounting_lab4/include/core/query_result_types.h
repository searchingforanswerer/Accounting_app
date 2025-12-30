#ifndef ACCOUNTING_CORE_QUERY_RESULT_TYPES_H_
#define ACCOUNTING_CORE_QUERY_RESULT_TYPES_H_

#include <vector>
#include <string>

namespace accounting {

// ============ 第三阶段：查询结果类型 ============

/**
 * @brief 分页查询结果
 * 
 * 用于返回分页的账单/交易数据
 */
template<typename T>
struct PagedResult {
    std::vector<T> items;           // 当前页的数据项
    int total_count = 0;            // 总记录数
    int page_number = 1;            // 当前页码（从 1 开始）
    int page_size = 10;             // 每页大小
    int total_pages = 0;            // 总页数

    /**
     * @brief 计算总页数（内部使用）
     */
    void CalculateTotalPages() {
        total_pages = (total_count + page_size - 1) / page_size;
    }

    /**
     * @brief 是否有下一页
     */
    bool HasNextPage() const {
        return page_number < total_pages;
    }

    /**
     * @brief 是否有上一页
     */
    bool HasPreviousPage() const {
        return page_number > 1;
    }
};

// ============ 第四阶段：预算分析结果类型 ============

/**
 * @brief 预算总体状态
 * 
 * 反映当前用户的总预算使用情况
 */
struct BudgetStatus {
    double total_budget = 0.0;          // 设置的总预算
    double used_amount = 0.0;           // 已使用的金额
    double remaining_budget = 0.0;      // 剩余预算
    double usage_percentage = 0.0;      // 使用百分比（0.0 ~ 1.0）
    bool is_exceeded = false;           // 是否已超过总预算
    bool budget_set = false;            // 是否已设置预算

    /**
     * @brief 检查是否接近预算上限（超过 80%）
     */
    bool IsNearLimit() const {
        return usage_percentage >= 0.8;
    }
};

/**
 * @brief 单个分类的预算状态
 * 
 * 反映某个分类的预算使用情况
 */
struct CategoryBudgetStatus {
    int category_id = -1;               // 分类 ID
    std::string category_name;          // 分类名称
    double limit = 0.0;                 // 分类预算限额
    double used = 0.0;                  // 已使用金额
    double remaining = 0.0;             // 剩余金额
    double usage_percentage = 0.0;      // 使用百分比（0.0 ~ 1.0）
    bool is_exceeded = false;           // 是否已超过预算
    bool limit_set = false;             // 是否设置了预算限额

    /**
     * @brief 检查是否接近预算上限（超过 80%）
     */
    bool IsNearLimit() const {
        return usage_percentage >= 0.8;
    }
};

/**
 * @brief 添加账单后的预算影响分析
 * 
 * 用于 UI 在用户添加账单前预知其对预算的影响
 */
struct BudgetImpact {
    bool would_exceed_total = false;        // 是否会超过总预算
    bool would_exceed_category = false;     // 是否会超过分类预算
    
    double current_remaining_total = 0.0;   // 当前剩余总预算
    double remaining_total_after_add = 0.0; // 添加后剩余总预算
    
    double current_remaining_category = 0.0;    // 当前剩余分类预算
    double remaining_category_after_add = 0.0;  // 添加后剩余分类预算
    
    std::string warning_message;            // 警告信息（供 UI 显示）

    /**
     * @brief 是否有任何预算风险
     */
    bool HasBudgetRisk() const {
        return would_exceed_total || would_exceed_category;
    }
};

}  // namespace accounting

#endif  // ACCOUNTING_CORE_QUERY_RESULT_TYPES_H_
