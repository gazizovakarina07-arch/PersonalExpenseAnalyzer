#include "personal_expense/application/ReportGenerator.h"
#include <limits>
#include <stdexcept>

namespace personal_expense::application
{
namespace
{
void addChecked(std::int64_t& total, std::int64_t amount)
{
    // Проверяем ДО сложения; оба числа неотрицательны.
    if (amount > std::numeric_limits<std::int64_t>::max() - total)
        throw std::overflow_error("Итог отчёта превышает диапазон денежных сумм");
    total += amount;
}
void addCategory(CategoryTotals& totals, const domain::Transaction& transaction,
    std::int64_t amount)
{
    if (transaction.hasCategory())
        addChecked(totals.named[transaction.category()->name()], amount);
    else
    {
        addChecked(totals.uncategorized, amount);
        ++totals.uncategorizedCount;
    }
}
}
MonthlyReport ReportGenerator::generate(
    const std::vector<domain::Transaction>& transactions, int year, int month) const
{
    const domain::Date firstDay(year, month, 1); // Проверка периода.
    MonthlyReport report{firstDay.year(), firstDay.month()};
    for (const auto& transaction : transactions)
    {
        if (transaction.date().year() != year || transaction.date().month() != month)
            continue;
        ++report.transactionCount;
        const auto amount = transaction.amountCents();
        if (transaction.isIncome())
        {
            addChecked(report.income, amount);
            addCategory(report.incomeCategories, transaction, amount);
        }
        else
        {
            // INT64_MIN нельзя безопасно отрицать.
            if (amount == std::numeric_limits<std::int64_t>::min())
                throw std::overflow_error("Расход слишком велик для положительного итога отчёта");
            addChecked(report.expenses, -amount);
            addCategory(report.expenseCategories, transaction, -amount);
        }
    }
    // Разность двух чисел из [0, INT64_MAX] всегда представима.
    report.difference = report.income - report.expenses;
    return report;
}
}
