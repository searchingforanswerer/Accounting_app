#ifndef ACCOUNTING_MODELS_BILL_DATA_H_
#define ACCOUNTING_MODELS_BILL_DATA_H_

#include <chrono>
#include <string>

namespace accounting {

class BillData {
public:
    // 默认构造
    BillData();

    // 带参构造
    BillData(double amount, const std::string& category_name,
             const std::string& category_type,
             const std::chrono::system_clock::time_point& time,
             const std::string& content);

    // 拷贝与赋值默认即可
    BillData(const BillData&) = default;
    BillData& operator=(const BillData&) = default;

    // Getter / Setter
    double GetAmount() const;
    void SetAmount(double amount);

    const std::string& GetCategoryName() const;
    void SetCategoryName(const std::string& category_name);

    const std::string& GetCategoryType() const;
    void SetCategoryType(const std::string& category_type);

    std::chrono::system_clock::time_point GetTime() const;
    void SetTime(const std::chrono::system_clock::time_point& time);

    const std::string& GetContent() const;
    void SetContent(const std::string& content);

    // 调试输出
    std::string ToString() const;

private:
    double amount_;
    std::string category_name_;
    std::string category_type_;  // 分类类型 ("expense", "income", 等)
    std::chrono::system_clock::time_point time_;
    std::string content_;
};

}  // namespace accounting

#endif  // ACCOUNTING_MODELS_BILL_DATA_H_
