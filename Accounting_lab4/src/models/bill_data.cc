#include "models/bill_data.h"
#include <iomanip>
#include <sstream>

namespace accounting {

BillData::BillData()
    : amount_(0.0),
      category_name_(""),
      time_(std::chrono::system_clock::now()),
      content_("") {}

BillData::BillData(double amount, const std::string& category_name,
                   const std::chrono::system_clock::time_point& time,
                   const std::string& content)
    : amount_(amount),
      category_name_(category_name),
      time_(time),
      content_(content) {}

double BillData::GetAmount() const { return amount_; }
void BillData::SetAmount(double amount) { amount_ = amount; }

const std::string& BillData::GetCategoryName() const { return category_name_; }
void BillData::SetCategoryName(const std::string& category_name) { 
    category_name_ = category_name; 
}

std::chrono::system_clock::time_point BillData::GetTime() const { return time_; }
void BillData::SetTime(const std::chrono::system_clock::time_point& time) { 
    time_ = time; 
}

const std::string& BillData::GetContent() const { return content_; }
void BillData::SetContent(const std::string& content) { content_ = content; }

std::string BillData::ToString() const {
    std::ostringstream oss;
    std::time_t t = std::chrono::system_clock::to_time_t(time_);
    oss << "BillData(Amount: " << amount_
        << ", Category: " << category_name_
        << ", Time: " << std::put_time(std::localtime(&t), "%F %T")
        << ", Content: " << content_
        << ")";
    return oss.str();
}

}  // namespace accounting