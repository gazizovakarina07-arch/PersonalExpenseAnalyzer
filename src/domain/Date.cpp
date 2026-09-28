#include "personal_expense/domain/Date.h"

#include <array>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace personal_expense::domain
{

namespace
{

bool isLeapYear(int year) noexcept
{
    return year % 400 == 0 ||
        (year % 4 == 0 && year % 100 != 0);
}

int daysInMonth(int year, int month) noexcept
{
    constexpr std::array<int, 12> days{
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    if (month == 2 && isLeapYear(year))
    {
        return 29;
    }

    return days[month - 1];
}

}

Date::Date(int year, int month, int day)
    : year_(year),
      month_(month),
      day_(day)
{
    if (year_ < 1 || year_ > 9999)
    {
        throw std::invalid_argument(
            "Год должен быть от 1 до 9999");
    }

    if (month_ < 1 || month_ > 12)
    {
        throw std::invalid_argument(
            "Месяц должен быть от 1 до 12");
    }

    if (day_ < 1 || day_ > daysInMonth(year_, month_))
    {
        throw std::invalid_argument(
            "День не существует в указанном месяце");
    }
}

int Date::year() const noexcept
{
    return year_;
}

int Date::month() const noexcept
{
    return month_;
}

int Date::day() const noexcept
{
    return day_;
}

std::string Date::toIsoString() const
{
    std::ostringstream output;

    output << std::setfill('0')
           << std::setw(4) << year_
           << '-'
           << std::setw(2) << month_
           << '-'
           << std::setw(2) << day_;

    return output.str();
}

}
