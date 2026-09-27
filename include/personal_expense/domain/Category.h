#pragma once

#include <string>

namespace personal_expense::domain
{

enum class CategoryType
{
    Income,
    Expense
};

class Category
{
public:
    Category(std::string name, CategoryType type);

    const std::string& name() const noexcept;
    CategoryType type() const noexcept;

private:
    std::string name_;
    CategoryType type_;
};

}
