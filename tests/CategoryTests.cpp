#include "personal_expense/domain/Category.h"

#include <iostream>
#include <stdexcept>

int main()
{
    using personal_expense::domain::Category;
    using personal_expense::domain::CategoryType;

    const Category category(
        "Продукты",
        CategoryType::Expense);

    if (category.name() != "Продукты")
    {
        std::cerr << "FAILED: category name is incorrect\n";
        return 1;
    }

    if (category.type() != CategoryType::Expense)
    {
        std::cerr << "FAILED: category type is incorrect\n";
        return 1;
    }

    bool emptyNameRejected = false;

    try
    {
        const Category emptyCategory(
            "",
            CategoryType::Expense);
    }
    catch (const std::invalid_argument&)
    {
        emptyNameRejected = true;
    }

    if (!emptyNameRejected)
    {
        std::cerr << "FAILED: empty category name was accepted\n";
        return 1;
    }

    bool whitespaceNameRejected = false;

    try
    {
        const Category whitespaceCategory(
            "   ",
            CategoryType::Expense);
    }
    catch (const std::invalid_argument&)
    {
        whitespaceNameRejected = true;
    }

    if (!whitespaceNameRejected)
    {
        std::cerr
            << "FAILED: whitespace-only category name was accepted\n";
        return 1;
    }

    std::cout << "PASSED: all Category tests\n";
    return 0;
}
