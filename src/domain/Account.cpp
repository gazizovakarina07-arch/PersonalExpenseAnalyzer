#include "personal_expense/domain/Account.h"

#include <stdexcept>
#include <utility>

namespace personal_expense::domain
{

Account::Account(std::string name)
    : name_(std::move(name))
{
    if (name_.find_first_not_of(" \t\n\r") == std::string::npos)
    {
        throw std::invalid_argument(
            "Название счёта не должно быть пустым");
    }
}

const std::string& Account::name() const noexcept
{
    return name_;
}

}
