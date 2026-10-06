#include "personal_expense/domain/Account.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace
{

bool isRejected(const std::string& name)
{
    try
    {
        const personal_expense::domain::Account account(name);
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
    using personal_expense::domain::Account;

    const Account account("Основная карта");

    if (account.name() != "Основная карта")
    {
        std::cerr << "FAILED: Account stores incorrect name\n";
        return 1;
    }

    if (!isRejected(""))
    {
        std::cerr << "FAILED: empty account name was accepted\n";
        return 1;
    }

    if (!isRejected("   "))
    {
        std::cerr
            << "FAILED: whitespace-only account name was accepted\n";
        return 1;
    }

    std::cout << "PASSED: all Account tests\n";
    return 0;
}
