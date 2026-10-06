#include "personal_expense/domain/Transaction.h"

#include <iostream>
#include <stdexcept>

namespace
{

using personal_expense::domain::Account;
using personal_expense::domain::Category;
using personal_expense::domain::CategoryType;
using personal_expense::domain::Date;
using personal_expense::domain::Transaction;

// Проверяем, отклоняется ли транзакция с пустым описанием.
bool rejectsBlankDescription()
{
    try
    {
        const Transaction transaction(
            Date(2026, 9, 29),
            "   ",
            -65000,
            Account("Основная карта"));
    }
    catch (const std::invalid_argument&)
    {
        return true;
    }

    return false;
}

// Проверяем, отклоняется ли операция с нулевой суммой.
bool rejectsZeroAmount()
{
    try
    {
        const Transaction transaction(
            Date(2026, 9, 29),
            "Операция с нулевой суммой",
            0,
            Account("Основная карта"));
    }
    catch (const std::invalid_argument&)
    {
        return true;
    }

    return false;
}

// Проверяем, что расход нельзя отнести к категории доходов.
bool rejectsIncomeCategoryForExpense()
{
    try
    {
        const Transaction transaction(
            Date(2026, 9, 29),
            "Покупка продуктов",
            -65000,
            Account("Основная карта"),
            Category("Зарплата", CategoryType::Income));
    }
    catch (const std::invalid_argument&)
    {
        return true;
    }

    return false;
}

// Проверяем, что доход нельзя отнести к категории расходов.
bool rejectsExpenseCategoryForIncome()
{
    try
    {
        const Transaction transaction(
            Date(2026, 9, 29),
            "Зачисление зарплаты",
            250000,
            Account("Основная карта"),
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
    // Создаём корректный расход с категорией.
    const Transaction expense(
        Date(2026, 9, 29),
        "Покупка продуктов",
        -65000,
        Account("Основная карта"),
        Category("Продукты", CategoryType::Expense));

    // Проверяем дату.
    if (expense.date().toIsoString() != "2026-09-29")
    {
        std::cerr << "FAILED: Transaction stores incorrect date\n";
        return 1;
    }

    // Проверяем описание.
    if (expense.description() != "Покупка продуктов")
    {
        std::cerr << "FAILED: Transaction stores incorrect description\n";
        return 1;
    }

    // Проверяем сумму в копейках.
    if (expense.amountCents() != -65000)
    {
        std::cerr << "FAILED: Transaction stores incorrect amount\n";
        return 1;
    }

    // Проверяем счёт.
    if (expense.account().name() != "Основная карта")
    {
        std::cerr << "FAILED: Transaction stores incorrect account\n";
        return 1;
    }

    // Отрицательная сумма должна определяться как расход.
    if (!expense.isExpense() || expense.isIncome())
    {
        std::cerr << "FAILED: negative amount is not an expense\n";
        return 1;
    }

    // У расхода должна быть категория.
    if (!expense.hasCategory())
    {
        std::cerr << "FAILED: Transaction lost its category\n";
        return 1;
    }

    // Проверяем название категории.
    //
    // Оператор -> позволяет обратиться к объекту Category,
    // который находится внутри std::optional.
    if (expense.category()->name() != "Продукты")
    {
        std::cerr << "FAILED: Transaction stores incorrect category\n";
        return 1;
    }

    // Создаём корректный доход без категории.
    const Transaction income(
        Date(2026, 9, 30),
        "Зачисление зарплаты",
        250000,
        Account("Основная карта"));

    // Положительная сумма должна определяться как доход.
    if (!income.isIncome() || income.isExpense())
    {
        std::cerr << "FAILED: positive amount is not an income\n";
        return 1;
    }

    // Отсутствующая категория является допустимым состоянием.
    if (income.hasCategory())
    {
        std::cerr << "FAILED: uncategorized Transaction has a category\n";
        return 1;
    }

    // Создаём расход без категории.
    Transaction uncategorizedExpense(
        Date(2026, 9, 30),
        "Поездка на автобусе",
        -6000,
        Account("Основная карта"));

    // Назначаем ему категорию после создания.
    uncategorizedExpense.assignCategory(
        Category("Транспорт", CategoryType::Expense));

    if (!uncategorizedExpense.hasCategory()
        || uncategorizedExpense.category()->name() != "Транспорт")
    {
        std::cerr << "FAILED: category was not assigned\n";
        return 1;
    }

    // Удаляем назначенную категорию.
    uncategorizedExpense.clearCategory();

    if (uncategorizedExpense.hasCategory())
    {
        std::cerr << "FAILED: category was not cleared\n";
        return 1;
    }

    // Проверяем все ошибочные варианты.
    if (!rejectsBlankDescription())
    {
        std::cerr << "FAILED: blank description was accepted\n";
        return 1;
    }

    if (!rejectsZeroAmount())
    {
        std::cerr << "FAILED: zero amount was accepted\n";
        return 1;
    }

    if (!rejectsIncomeCategoryForExpense())
    {
        std::cerr
            << "FAILED: income category was accepted for expense\n";
        return 1;
    }

    if (!rejectsExpenseCategoryForIncome())
    {
        std::cerr
            << "FAILED: expense category was accepted for income\n";
        return 1;
    }

    // Эта строка выполняется только в том случае,
    // если все предыдущие проверки были успешными.
    std::cout << "PASSED: all Transaction tests\n";
    return 0;
}
