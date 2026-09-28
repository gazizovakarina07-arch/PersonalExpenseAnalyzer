#include "personal_expense/domain/Date.h"

#include <iostream>
#include <stdexcept>

namespace
{

bool isRejected(int year, int month, int day)
{
    try
    {
        const personal_expense::domain::Date date(
            year,
            month,
            day);
    }
    catch (const std::invalid_argument&)
    {
        return true;
    }

    return false;
}

}

int main()
{
    using personal_expense::domain::Date;

    const Date date(2026, 9, 8);

    if (date.year() != 2026 ||
        date.month() != 9 ||
        date.day() != 8)
    {
        std::cerr << "FAILED: Date stores incorrect values\n";
        return 1;
    }

    if (date.toIsoString() != "2026-09-08")
    {
        std::cerr << "FAILED: incorrect ISO date\n";
        return 1;
    }

    const Date leapYearDate(2024, 2, 29);
    const Date leapCenturyDate(2000, 2, 29);

    if (leapYearDate.day() != 29 ||
        leapCenturyDate.day() != 29)
    {
        std::cerr << "FAILED: valid leap date was rejected\n";
        return 1;
    }

    if (!isRejected(2025, 2, 29))
    {
        std::cerr << "FAILED: invalid leap date was accepted\n";
        return 1;
    }

    if (!isRejected(1900, 2, 29))
    {
        std::cerr << "FAILED: year 1900 was treated as leap\n";
        return 1;
    }

    if (!isRejected(2026, 2, 31))
    {
        std::cerr << "FAILED: impossible February date was accepted\n";
        return 1;
    }

    if (!isRejected(2026, 13, 1))
    {
        std::cerr << "FAILED: invalid month was accepted\n";
        return 1;
    }

    if (!isRejected(0, 1, 1))
    {
        std::cerr << "FAILED: invalid year was accepted\n";
        return 1;
    }

    std::cout << "PASSED: all Date tests\n";
    return 0;
}
