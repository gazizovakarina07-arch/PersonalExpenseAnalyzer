#include "personal_expense/presentation/ReportPrinter.h"
#include <iomanip>
#include <ostream>

namespace personal_expense::presentation
{
std::string formatMoney(std::int64_t cents)
{
    // Без double; безопасно даже для минимального int64_t.
    const auto magnitude = cents < 0
        ? static_cast<std::uint64_t>(-(cents + 1)) + 1
        : static_cast<std::uint64_t>(cents);
    std::string rubles = std::to_string(magnitude / 100);
    for (std::size_t i = rubles.size(); i > 3; i -= 3)
        rubles.insert(i - 3, " ");
    const auto remainder = magnitude % 100;
    return (cents < 0 ? "-" : "") + rubles + ','
        + (remainder < 10 ? "0" : "") + std::to_string(remainder);
}
namespace
{
void printCategories(std::ostream& output, const application::CategoryTotals& totals)
{
    if (totals.named.empty() && totals.uncategorizedCount == 0)
        output << "  Нет операций.\n";
    for (const auto& entry : totals.named)
    {
        output << "  " << entry.first;
        if (entry.first == "Без категории") output << " (название категории)";
        output << ": " << formatMoney(entry.second) << " ₽\n";
    }
    // Отсутствие категории не смешивается с настоящими категориями и идёт последним.
    if (totals.uncategorizedCount != 0)
        output << "  Без категории: " << formatMoney(totals.uncategorized) << " ₽\n";
}
}
void printReport(std::ostream& output, const application::MonthlyReport& report)
{
    output << "\nОтчёт за " << std::setfill('0') << std::setw(4) << report.year
        << '-' << std::setw(2) << report.month << std::setfill(' ') << '\n'
        << "Операций: " << report.transactionCount << '\n'
        << "Доходы: " << formatMoney(report.income) << " ₽\n"
        << "Расходы: " << formatMoney(report.expenses) << " ₽\n"
        << "Разница: " << formatMoney(report.difference) << " ₽\n"
        << "Разница доходов и расходов не является остатком на счёте.\n"
        << "\nДоходы по категориям:\n";
    printCategories(output, report.incomeCategories);
    output << "\nРасходы по категориям:\n";
    printCategories(output, report.expenseCategories);
}
}
