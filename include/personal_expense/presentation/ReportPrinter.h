#pragma once
#include "personal_expense/application/ReportGenerator.h"
#include <iosfwd>
#include <string>
namespace personal_expense::presentation
{
std::string formatMoney(std::int64_t cents);
void printReport(std::ostream& output, const application::MonthlyReport& report);
}
