#include "personal_expense/domain/KeywordCategorizationRule.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{

using namespace personal_expense::domain;

// Создаёт тестовую операцию.
// Остальные данные одинаковые, чтобы в проверках
// сосредоточиться на описании и сумме.
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

// Проверяет, запрещено ли указанное ключевое слово.
bool rejectsKeyword(const std::string& keyword)
{
    try
    {
        const KeywordCategorizationRule rule(
            keyword,
            Category("Продукты", CategoryType::Expense));
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
    // Соответствие магазину задаём явно.
    const KeywordCategorizationRule rule(
        "Пятерочка",
        Category("Продукты", CategoryType::Expense));

    // Работаем с конкретным правилом через общий интерфейс.
    // Это проверка полиморфного вызова matches().
    const ICategorizationRule& interface = rule;

    const Transaction purchase =
        makeTransaction("Пятерочка", -65000);

    if (!interface.matches(purchase))
    {
        std::cerr << "FAILED: exact keyword was not matched\n";
        return 1;
    }

    // Ключевое слово может быть частью более длинного описания.
    if (!interface.matches(
            makeTransaction("Оплата: Пятерочка №123", -65000)))
    {
        std::cerr << "FAILED: keyword inside description was not matched\n";
        return 1;
    }

    // Незнакомый продавец не должен подходить под правило.
    if (interface.matches(makeTransaction("ИП Иванов", -12000)))
    {
        std::cerr << "FAILED: unrelated description was matched\n";
        return 1;
    }

    // Фиксируем выбранное ограничение: регистр учитывается.
    if (interface.matches(makeTransaction("пятерочка", -65000)))
    {
        std::cerr << "FAILED: letter case was ignored\n";
        return 1;
    }

    // Расходное правило не должно подходить к доходу,
    // даже если описание совпало.
    if (interface.matches(makeTransaction("Пятерочка", 65000)))
    {
        std::cerr << "FAILED: expense rule matched income\n";
        return 1;
    }

    // Правило должно предоставлять заданную категорию.
    if (interface.category().name() != "Продукты"
        || interface.category().type() != CategoryType::Expense)
    {
        std::cerr << "FAILED: incorrect category returned\n";
        return 1;
    }

    // Проверка совпадения не должна назначать категорию сама.
    if (purchase.hasCategory())
    {
        std::cerr << "FAILED: matching changed the transaction\n";
        return 1;
    }

    // Проверяем также правило для доходов.
    const KeywordCategorizationRule incomeRule(
        "Зарплата",
        Category("Зарплата", CategoryType::Income));

    if (!incomeRule.matches(makeTransaction("Зарплата", 4500000)))
    {
        std::cerr << "FAILED: income rule did not match income\n";
        return 1;
    }

    if (incomeRule.matches(makeTransaction("Зарплата", -4500000)))
    {
        std::cerr << "FAILED: income rule matched expense\n";
        return 1;
    }

    // Пустой поиск и поиск одних пробельных символов запрещены.
    if (!rejectsKeyword("") || !rejectsKeyword(" \t\n\r"))
    {
        std::cerr << "FAILED: blank keyword was accepted\n";
        return 1;
    }

    std::cout << "PASSED: all KeywordCategorizationRule tests\n";
    return 0;
}
