#include "personal_expense/application/ReportGenerator.h"
#include "personal_expense/presentation/ReportPrinter.h"
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace
{
using namespace personal_expense;
using namespace domain;
void require(bool value, const char* message)
{
    if (!value) throw std::runtime_error(message);
}
Transaction operation(int year, int month, std::int64_t amount,
    std::optional<Category> category = std::nullopt)
{
    return Transaction(Date(year, month, 1), "Операция", amount, Account("Карта"), category);
}
void check()
{
    application::ReportGenerator generator;
    const std::vector<Transaction> transactions{
        operation(2026, 10, 4500000, Category("Зарплата", CategoryType::Income)),
        operation(2026, 10, -65000, Category("Продукты", CategoryType::Expense)),
        operation(2026, 10, -38050, Category("Здоровье", CategoryType::Expense)),
        operation(2026, 10, -12000), operation(2026, 9, -999),
        operation(2025, 10, 999)};
    const auto report = generator.generate(transactions, 2026, 10);
    require(report.transactionCount == 4 && report.income == 4500000
        && report.expenses == 115050 && report.difference == 4384950, "incorrect monthly totals");
    require(report.expenseCategories.named.at("Продукты") == 65000
        && report.expenseCategories.uncategorized == 12000, "incorrect grouping");
    std::ostringstream output;
    presentation::printReport(output, report);
    require(output.str().find("43 849,50 ₽") != std::string::npos, "wrong Russian money output");
    require(output.str().rfind("Без категории: 120,00 ₽\n")
        + std::string("Без категории: 120,00 ₽\n").size() == output.str().size(), "unknown category not last");

    const auto empty = generator.generate(transactions, 2026, 11);
    require(empty.transactionCount == 0 && empty.income == 0 && empty.expenses == 0,
        "empty month has totals");
    require(generator.generate({}, 2026, 10).transactionCount == 0, "empty input failed");
    const auto loss = generator.generate({operation(2026, 10, -1)}, 2026, 10);
    require(loss.difference == -1, "negative difference failed");
    const auto mixed = generator.generate({operation(2026, 10, 100),
        operation(2026, 10, -200), operation(2026, 10, -300,
            Category("Без категории", CategoryType::Expense)),
        operation(2026, 10, -400, Category("Без категории", CategoryType::Expense))}, 2026, 10);
    require(mixed.incomeCategories.uncategorized == 100
        && mixed.expenseCategories.uncategorized == 200
        && mixed.expenseCategories.named.at("Без категории") == 700, "category collision or aggregation");
    for (const auto period : {std::pair<int,int>{0, 1}, {2026, 0}, {2026, 13}})
    {
        bool rejected = false;
        try { generator.generate({}, period.first, period.second); }
        catch (const std::invalid_argument&) { rejected = true; }
        require(rejected, "invalid period accepted");
    }
    const auto maximum = std::numeric_limits<std::int64_t>::max();
    const auto minimum = std::numeric_limits<std::int64_t>::min();
    for (const auto amounts : {std::vector<std::int64_t>{maximum, 1}, {-maximum, -1}, {minimum}})
    {
        std::vector<Transaction> values;
        for (auto amount : amounts) values.push_back(operation(2026, 10, amount));
        bool rejected = false;
        try { generator.generate(values, 2026, 10); }
        catch (const std::overflow_error&) { rejected = true; }
        require(rejected, "overflow not detected");
    }
    require(generator.generate({operation(2026, 10, maximum), operation(2026, 10, -maximum)},
        2026, 10).difference == 0, "boundary totals rejected");
    require(presentation::formatMoney(0) == "0,00"
        && presentation::formatMoney(-1) == "-0,01"
        && presentation::formatMoney(minimum) == "-92 233 720 368 547 758,08", "format boundary failed");
}
}
int main()
{
    try { check(); std::cout << "PASSED: report tests\n"; return 0; }
    catch (const std::exception& e) { std::cerr << "FAILED: " << e.what() << '\n'; return 1; }
}
