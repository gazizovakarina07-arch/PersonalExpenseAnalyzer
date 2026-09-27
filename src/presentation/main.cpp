#include "personal_expense/domain/Category.h"

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

int main()
{
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    const personal_expense::domain::Category category(
        "Продукты",
        personal_expense::domain::CategoryType::Expense);

    std::cout << "Personal Expense Analyzer\n";
    std::cout << "Категория: " << category.name() << '\n';

    return 0;
}
