#include "personal_expense/domain/ExactDescriptionRule.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{

using namespace personal_expense::domain;

// Создаёт операцию для проверки правила.
Transaction makeTransaction(
    const std::string& description,
    std::int64_t amountCents)
{
    return Transaction(
        Date(2026, 10, 1),
        description,
        amountCents,
        Account("Основная карта"));
}

// Проверяет, отклоняется ли недопустимая настройка правила.
bool rejectsDescription(const std::string& description)
{
    try
    {
        const ExactDescriptionRule rule(
            description,
            Category("Зарплата", CategoryType::Income));
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
    const ExactDescriptionRule rule(
        "Зачисление зарплаты",
        Category("Зарплата", CategoryType::Income));

    // Обращаемся к конкретному правилу через общий интерфейс.
    const ICategorizationRule& interface = rule;

    const Transaction income =
        makeTransaction("Зачисление зарплаты", 4500000);

    // Полное совпадение должно подходить.
    if (!interface.matches(income))
    {
        std::cerr << "FAILED: exact description was not matched\n";
        return 1;
    }

    // Дополнительный текст должен нарушать совпадение.
    // Эта проверка отличает точное правило от поиска подстроки.
    if (interface.matches(
            makeTransaction("Зачисление зарплаты и премии", 4500000)))
    {
        std::cerr << "FAILED: extended description was matched\n";
        return 1;
    }

    // Проверяем описание, отличающееся регистром.
    if (interface.matches(
            makeTransaction("зачисление зарплаты", 4500000)))
    {
        std::cerr << "FAILED: letter case was ignored\n";
        return 1;
    }

    // Даже пробел в конце делает строки разными.
    if (interface.matches(
            makeTransaction("Зачисление зарплаты ", 4500000)))
    {
        std::cerr << "FAILED: trailing space was ignored\n";
        return 1;
    }

    // Доходное правило не должно подходить расходу.
    if (interface.matches(
            makeTransaction("Зачисление зарплаты", -4500000)))
    {
        std::cerr << "FAILED: income rule matched expense\n";
        return 1;
    }

    // Проверяем категорию, предоставляемую правилом.
    if (interface.category().name() != "Зарплата"
        || interface.category().type() != CategoryType::Income)
    {
        std::cerr << "FAILED: incorrect category returned\n";
        return 1;
    }

    // Проверка правила сама не назначает категорию.
    if (income.hasCategory())
    {
        std::cerr << "FAILED: matching changed the transaction\n";
        return 1;
    }

    // Проверяем работу правила для расхода.
    const ExactDescriptionRule expenseRule(
        "Оплата проезда",
        Category("Транспорт", CategoryType::Expense));

    if (!expenseRule.matches(
            makeTransaction("Оплата проезда", -6000)))
    {
        std::cerr << "FAILED: expense rule did not match expense\n";
        return 1;
    }

    if (expenseRule.matches(
            makeTransaction("Оплата проезда", 6000)))
    {
        std::cerr << "FAILED: expense rule matched income\n";
        return 1;
    }

    // Пустые и состоящие из пробельных символов настройки запрещены.
    if (!rejectsDescription("")
        || !rejectsDescription(" \t\n\r"))
    {
        std::cerr << "FAILED: blank description was accepted\n";
        return 1;
    }

    std::cout << "PASSED: all ExactDescriptionRule tests\n";
    return 0;
}
