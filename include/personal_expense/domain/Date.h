#pragma once

#include <string>

namespace personal_expense::domain
{

class Date
{
public:
    Date(int year, int month, int day);

    int year() const noexcept;
    int month() const noexcept;
    int day() const noexcept;

    std::string toIsoString() const;

private:
    int year_;
    int month_;
    int day_;
};

}
