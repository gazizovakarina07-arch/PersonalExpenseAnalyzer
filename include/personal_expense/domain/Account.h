#pragma once

#include <string>

namespace personal_expense::domain
{

class Account
{
public:
    explicit Account(std::string name);

    const std::string& name() const noexcept;

private:
    std::string name_;
};

}
