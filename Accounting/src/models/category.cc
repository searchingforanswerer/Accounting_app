#include "models/category.h"

namespace accounting {

Category::Category()
    : category_id_(0), name_(""), type_(""), color_("#FFFFFF") {}

Category::Category(int category_id, const std::string& name,
                   const std::string& type, const std::string& color)
    : category_id_(category_id), name_(name), type_(type), color_(color) {}

int Category::GetCategoryId() const { return category_id_; }
void Category::SetCategoryId(int id) { category_id_ = id; }

const std::string& Category::GetName() const { return name_; }
void Category::SetName(const std::string& name) { name_ = name; }

const std::string& Category::GetType() const { return type_; }
void Category::SetType(const std::string& type) { type_ = type; }

const std::string& Category::GetColor() const { return color_; }
void Category::SetColor(const std::string& color) { color_ = color; }

bool Category::operator==(const Category& other) const {
  return category_id_ == other.category_id_;
}

bool Category::operator<(const Category& other) const {
  return category_id_ < other.category_id_;
}

std::string Category::ToString() const {
  return "Category(ID: " + std::to_string(category_id_) + ", Name: " + name_ +
         ", Type: " + type_ + ", Color: " + color_ + ")";
}

// ==== JSON Support ====
void to_json(json& j, const Category& c) {
    j = json{
        {"category_id", c.category_id_},
        {"name", c.name_},
        {"type", c.type_},
        {"color", c.color_}
    };
}

void from_json(const json& j, Category& c) {
    c.category_id_ = j.at("category_id").get<int>();
    c.name_ = j.at("name").get<std::string>();
    c.type_ = j.at("type").get<std::string>();
    c.color_ = j.at("color").get<std::string>();
}

}  // namespace accounting
