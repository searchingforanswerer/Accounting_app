#ifndef ACCOUNTING_MODELS_QUERY_CRITERIA_H_
#define ACCOUNTING_MODELS_QUERY_CRITERIA_H_

#include <chrono>
#include <string>

namespace accounting {

class QueryCriteria {
public:
    // 默认构造
    QueryCriteria();

    // 带参构造
    QueryCriteria(const std::chrono::system_clock::time_point& start_date,
                  const std::chrono::system_clock::time_point& end_date,
                  const std::string& category_name);

    // 拷贝与赋值默认即可
    QueryCriteria(const QueryCriteria&) = default;
    QueryCriteria& operator=(const QueryCriteria&) = default;

    // Getter / Setter
    std::chrono::system_clock::time_point GetStartDate() const;
    void SetStartDate(const std::chrono::system_clock::time_point& start_date);

    std::chrono::system_clock::time_point GetEndDate() const;
    void SetEndDate(const std::chrono::system_clock::time_point& end_date);

    const std::string& GetCategoryName() const;
    void SetCategoryName(const std::string& category_name);

    // 便捷方法
    bool HasDateRange() const;
    bool HasCategoryFilter() const;

    // 调试输出
    std::string ToString() const;

private:
    std::chrono::system_clock::time_point start_date_;
    std::chrono::system_clock::time_point end_date_;
    std::string category_name_;
};

}  // namespace accounting

#endif  // ACCOUNTING_MODELS_QUERY_CRITERIA_H_