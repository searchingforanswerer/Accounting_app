#include "models/bill.h"
#include <iomanip>
#include <sstream>

namespace accounting {

// 默认构造
Bill::Bill()
    : bill_id_(0),
    amount_(0.0),
    category_(nullptr),
    category_id_(-1),
    time_(std::chrono::system_clock::now()),
    content_("") {}

// 带参构造
Bill::Bill(int bill_id, double amount,
           std::shared_ptr<Category> category,
           const std::chrono::system_clock::time_point& time,
           const std::string& content)
    : bill_id_(bill_id),
    amount_(amount),
    category_(category),
    category_id_((category) ? category->GetCategoryId() : -1),
    time_(time),
    content_(content) {}

int Bill::GetBillId() const { return bill_id_; }
void Bill::SetBillId(int id) { bill_id_ = id; }

double Bill::GetAmount() const { return amount_; }
void Bill::SetAmount(double amount) { amount_ = amount; }

std::shared_ptr<Category> Bill::GetCategory() const { return category_; }
void Bill::SetCategory(const std::shared_ptr<Category>& category) { 
    category_ = category; 
    category_id_ = category ? category->GetCategoryId() : -1;
}

int Bill::GetCategoryId() const { return category_id_; }

void Bill::SetCategoryId(int category_id) { category_id_ = category_id; }

std::chrono::system_clock::time_point Bill::GetTime() const { return time_; }
void Bill::SetTime(const std::chrono::system_clock::time_point& time) { time_ = time; }

const std::string& Bill::GetContent() const { return content_; }
void Bill::SetContent(const std::string& content) { content_ = content; }

// 调试输出
std::string Bill::ToString() const {
    std::ostringstream oss;
    // 将时间转换为可读字符串
    std::time_t t = std::chrono::system_clock::to_time_t(time_);
    oss << "Bill(ID: " << bill_id_
        << ", Amount: " << amount_;
    if (category_) {
        oss << ", Category: " << category_->GetName();
    } else {
        oss << ", Category: NULL";
    }
    oss << ", Time: " << std::put_time(std::localtime(&t), "%F %T")
        << ", Content: " << content_
        << ")";
    return oss.str();
}

// ===== JSON 序列化实现 =====

// 时间转换辅助函数
static std::string TimePointToString(const std::chrono::system_clock::time_point& tp) {
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::localtime(&t);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return buf;
}

static std::chrono::system_clock::time_point StringToTimePoint(const std::string& s) {
    std::tm tm{};
    std::istringstream ss(s);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto t = std::mktime(&tm);
    return std::chrono::system_clock::from_time_t(t);
}

void to_json(json& j, const Bill& b) {
    j = json{
        {"bill_id", b.bill_id_},
        {"amount", b.amount_},
        // 保存 category_id（优先使用 shared_ptr 的 id，否则使用存储的 category_id_）
        {"category_id", b.category_ ? b.category_->GetCategoryId() : b.category_id_},
        {"time", TimePointToString(b.time_)},
        {"content", b.content_}
    };
}

void from_json(const json& j, Bill& b) {
    b.bill_id_ = j.at("bill_id").get<int>();
    b.amount_ = j.at("amount").get<double>();
    b.content_ = j.at("content").get<std::string>();
    b.time_ = StringToTimePoint(j.at("time").get<std::string>());

    // 只读取 category_id，暂时不创建 category_ 指针（由 BillManager 恢复）
    int category_id = -1;
    if (j.contains("category_id") && !j.at("category_id").is_null()) {
        category_id = j.at("category_id").get<int>();
    }
    b.category_ = nullptr;
    b.category_id_ = category_id;

    // **可选**：临时把 category_id 存到 content 或其它地方以便后续恢复，
    // 但更清晰的做法是让 BillManager 在 Load 阶段读取 JSON -> Bill，
    // 然后再通过 CategoryManager 查找并设置 bill.SetCategory(...)
}


}  // namespace accounting
