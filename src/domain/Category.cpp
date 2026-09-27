#include "personal_expense/domain/Category.h"

#include <stdexcept>
#include <utility>

namespace personal_expense::domain
{

Category::Category(std::string name, CategoryType type)
    : name_(std::move(name)),
      type_(type)
{
    if (name_.find_first_not_of(" \t\n\r") == std::string::npos)
    {
        throw std::invalid_argument(
            "Название категории не должно быть пустым");
    }
}

const std::string& Category::name() const noexcept
{
    return name_;
}

CategoryType Category::type() const noexcept
{
    return type_;
}

}
