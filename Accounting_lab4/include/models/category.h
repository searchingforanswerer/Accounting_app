#ifndef ACCOUNTING_MODELS_CATEGORY_H_
#define ACCOUNTING_MODELS_CATEGORY_H_

#include <string>
#include <iostream>
#include <nlohmann/json.hpp>

namespace accounting {

using nlohmann::json;

class Category {
public:
    // 默认构造函数
    Category();

    // 带参构造函数
    Category(int category_id, const std::string& name,
            const std::string& type, const std::string& color);

    // 拷贝构造与赋值采用默认行为
    Category(const Category&) = default;
    Category& operator=(const Category&) = default;

    // Getter 与 Setter
    int GetCategoryId() const;
    void SetCategoryId(int id);

    const std::string& GetName() const;
    void SetName(const std::string& name);

    const std::string& GetType() const;
    void SetType(const std::string& type);

    const std::string& GetColor() const;
    void SetColor(const std::string& color);

    // 运算符重载（便于放入容器）
    bool operator==(const Category& other) const;
    bool operator<(const Category& other) const;

    // 调试用字符串输出
    std::string ToString() const;

private:
    int category_id_;         // 类别ID
    std::string name_;        // 类别名称
    std::string type_;        // 类别类型（如"expense"、"income"、"exorin"）
    std::string color_;       // 显示颜色（如"#FFAA00"）

    // 添加JSON序列化支持
    friend void to_json(json& j, const Category& c);
    friend void from_json(const json& j, Category& c);
};

}  // namespace accounting

#endif  // ACCOUNTING_MODELS_CATEGORY_H_
