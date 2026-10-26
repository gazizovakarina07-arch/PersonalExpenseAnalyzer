#include "personal_expense/application/TransactionCategorizer.h"
#include "personal_expense/domain/ExactDescriptionRule.h"
#include "personal_expense/domain/KeywordCategorizationRule.h"

#include <iostream>
#include <memory>
#include <stdexcept>

namespace
{
using namespace personal_expense::domain;
using personal_expense::application::TransactionCategorizer;

Transaction makeTransaction(const std::string& description, std::int64_t amount)
{
    return Transaction(Date(2026, 10, 1), description, amount,
        Account("Основная карта"));
}

// Проверки работают и в Release: они не зависят от assert/NDEBUG.
void require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

void checkCategorization()
{
    TransactionCategorizer categorizer;
    auto unknown = makeTransaction("ИП Иванов", -12000);
    require(!categorizer.categorize(unknown) && !unknown.hasCategory(),
        "empty rule list changed transaction");

    // Два разных вида правил хранятся через один интерфейс.
    categorizer.addRule(std::make_unique<KeywordCategorizationRule>(
        "Пятерочка", Category("Продукты", CategoryType::Expense)));
    categorizer.addRule(std::make_unique<ExactDescriptionRule>(
        "Зачисление зарплаты", Category("Зарплата", CategoryType::Income)));

    auto purchase = makeTransaction("Пятерочка №123", -65000);
    require(categorizer.categorize(purchase), "keyword rule was not applied");
    require(purchase.hasCategory() && purchase.category()->name() == "Продукты",
        "incorrect purchase category");
    require(purchase.amountCents() == -65000
        && purchase.description() == "Пятерочка №123"
        && purchase.date().toIsoString() == "2026-10-01"
        && purchase.account().name() == "Основная карта",
        "categorization changed other transaction data");
    require(!categorizer.categorize(purchase), "repeat call reassigned category");

    auto salary = makeTransaction("Зачисление зарплаты", 4500000);
    require(categorizer.categorize(salary), "exact rule was not applied");
    require(salary.hasCategory() && salary.category()->name() == "Зарплата",
        "incorrect salary category");

    require(!categorizer.categorize(unknown) && !unknown.hasCategory(),
        "unknown merchant received a category");
    auto refund = makeTransaction("Пятерочка", 65000);
    require(!categorizer.categorize(refund) && !refund.hasCategory(),
        "expense category assigned to income");

    auto manual = makeTransaction("Пятерочка", -65000);
    manual.assignCategory(Category("Дом", CategoryType::Expense));
    require(!categorizer.categorize(manual)
        && manual.category()->name() == "Дом", "manual category was replaced");

    // Оба правила подходят. Приоритет определяется порядком, не видом.
    TransactionCategorizer priority;
    priority.addRule(std::make_unique<ExactDescriptionRule>(
        "Пятерочка", Category("Дом", CategoryType::Expense)));
    priority.addRule(std::make_unique<KeywordCategorizationRule>(
        "Пятерочка", Category("Продукты", CategoryType::Expense)));
    auto first = makeTransaction("Пятерочка", -10000);
    require(priority.categorize(first) && first.category()->name() == "Дом",
        "first matching rule did not win");

    TransactionCategorizer reversed;
    reversed.addRule(std::make_unique<KeywordCategorizationRule>(
        "Пятерочка", Category("Продукты", CategoryType::Expense)));
    reversed.addRule(std::make_unique<ExactDescriptionRule>(
        "Пятерочка", Category("Дом", CategoryType::Expense)));
    auto second = makeTransaction("Пятерочка", -10000);
    require(reversed.categorize(second) && second.category()->name() == "Продукты",
        "rule type overrode insertion order");

    bool rejected = false;
    try
    {
        categorizer.addRule(nullptr);
    }
    catch (const std::invalid_argument&)
    {
        rejected = true;
    }
    require(rejected, "null rule was accepted");
    auto afterRejection = makeTransaction("Пятерочка", -10000);
    require(categorizer.categorize(afterRejection),
        "rejecting null damaged existing rules");
}
}

int main()
{
    try
    {
        checkCategorization();
        std::cout << "PASSED: all TransactionCategorizer tests\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAILED: " << error.what() << '\n';
        return 1;
    }
}
