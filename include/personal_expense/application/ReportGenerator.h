#pragma once
#include "personal_expense/domain/Transaction.h"
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace personal_expense::application
{
// Доходы и расходы группируются отдельно; суммы расходов положительные.
struct CategoryTotals
{
    std::map<std::string, std::int64_t> named;
    std::int64_t uncategorized = 0;
    std::size_t uncategorizedCount = 0;
};
struct MonthlyReport
{
    int year;
    int month;
    std::size_t transactionCount = 0;
    std::int64_t income = 0;
    std::int64_t expenses = 0;
    std::int64_t difference = 0;
    CategoryTotals incomeCategories;
    CategoryTotals expenseCategories;
};
class ReportGenerator
{
public:
    // Только расчёт. При переполнении положительных итогов бросает overflow_error.
    MonthlyReport generate(const std::vector<domain::Transaction>& transactions,
        int year, int month) const;
};
}
