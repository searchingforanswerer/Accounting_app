#include "models/query_criteria.h"
#include <iomanip>
#include <sstream>

namespace accounting {

QueryCriteria::QueryCriteria()
    : start_date_(std::chrono::system_clock::time_point::min()),
      end_date_(std::chrono::system_clock::time_point::max()),
      category_name_("") {}

QueryCriteria::QueryCriteria(const std::chrono::system_clock::time_point& start_date,
                             const std::chrono::system_clock::time_point& end_date,
                             const std::string& category_name)
    : start_date_(start_date),
      end_date_(end_date),
      category_name_(category_name) {}

std::chrono::system_clock::time_point QueryCriteria::GetStartDate() const { 
    return start_date_; 
}

void QueryCriteria::SetStartDate(const std::chrono::system_clock::time_point& start_date) {
    start_date_ = start_date;
}

std::chrono::system_clock::time_point QueryCriteria::GetEndDate() const { 
    return end_date_; 
}

void QueryCriteria::SetEndDate(const std::chrono::system_clock::time_point& end_date) {
    end_date_ = end_date;
}

const std::string& QueryCriteria::GetCategoryName() const { 
    return category_name_; 
}

void QueryCriteria::SetCategoryName(const std::string& category_name) {
    category_name_ = category_name;
}

bool QueryCriteria::HasDateRange() const {
    return start_date_ != std::chrono::system_clock::time_point::min() ||
           end_date_ != std::chrono::system_clock::time_point::max();
}

bool QueryCriteria::HasCategoryFilter() const {
    return !category_name_.empty();
}

std::string QueryCriteria::ToString() const {
    std::ostringstream oss;
    
    oss << "QueryCriteria(";
    
    if (HasDateRange()) {
        std::time_t start_t = std::chrono::system_clock::to_time_t(start_date_);
        std::time_t end_t = std::chrono::system_clock::to_time_t(end_date_);
        oss << "DateRange: [" << std::put_time(std::localtime(&start_t), "%F %T")
            << " to " << std::put_time(std::localtime(&end_t), "%F %T") << "]";
    }
    
    if (HasCategoryFilter()) {
        if (HasDateRange()) oss << ", ";
        oss << "Category: " << category_name_;
    }
    
    if (!HasDateRange() && !HasCategoryFilter()) {
        oss << "No filters";
    }
    
    oss << ")";
    
    return oss.str();
}

}  // namespace accounting