#include "personal_expense/application/ReportGenerator.h"
#include "personal_expense/application/TransactionCategorizer.h"
#include "personal_expense/domain/ExactDescriptionRule.h"
#include "personal_expense/domain/KeywordCategorizationRule.h"
#include "personal_expense/infrastructure/CsvStatementImporter.h"
#include "personal_expense/presentation/ReportPrinter.h"
#include "personal_expense/presentation/ConsoleMenu.h"
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace
{
int run(const std::filesystem::path& path, const std::string& period, bool reportOnly)
{
    using namespace personal_expense;
    using namespace personal_expense::domain;
    try
    {
        if (period.size() != 7 || period[4] != '-')
            throw std::invalid_argument("Месяц нужно указать в формате YYYY-MM");
        for (std::size_t i = 0; i < period.size(); ++i)
            if (i != 4 && (period[i] < '0' || period[i] > '9'))
                throw std::invalid_argument("Месяц нужно указать в формате YYYY-MM");
        const Date periodDate(std::stoi(period.substr(0, 4)),
            std::stoi(period.substr(5, 2)), 1);
        auto imported = infrastructure::CsvStatementImporter().importFile(path);
        // Итоги импорта относятся ко всему файлу, отчёт — к выбранному месяцу.
        std::cout << "Импортировано операций из файла: " << imported.transactions.size()
            << "\nОшибок импорта: " << imported.errors.size() << '\n';
        for (const auto& error : imported.errors)
            std::cout << "Строка " << error.line << ": " << error.message << '\n';
        if (!imported.errors.empty())
            std::cout << "Внимание: отчёт построен только по корректным строкам файла.\n";
        presentation::ConsoleMenu menu(imported.transactions, periodDate.year(), periodDate.month());
        std::cout << "Категоризация: встроенные правила для Пятерочки, Аптеки и зарплаты.\n";
        menu.applyInitialRules();
        const auto report = application::ReportGenerator().generate(
            imported.transactions, periodDate.year(), periodDate.month());
        presentation::printReport(std::cout, report);
        if (!reportOnly) menu.run(std::cin, std::cout);
        return imported.errors.empty() ? 0 : 2; // 2 — частичный импорт.
    }
    catch (const std::exception& error)
    {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }
}
int usage()
{
    std::cout << "Локальный анализатор расходов\n"
        << "Запуск: PersonalExpenseAnalyzer.exe <путь к CSV> <YYYY-MM> [--report]\n"
        << "Без --report открывается меню; с --report выводится только отчёт.\n"
        << "Пример: PersonalExpenseAnalyzer.exe data/demo_statement.csv 2026-10\n"
        << "Путь с пробелами заключите в двойные кавычки.\n";
    return 1;
}
}
#ifdef _WIN32
// Широкие аргументы сохраняют русские буквы в путях Windows.
int wmain(int argc, wchar_t* argv[])
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    if (argc != 3 && argc != 4) return usage();
    if (argc == 4 && std::wstring(argv[3]) != L"--report") return usage();
    const std::wstring period(argv[2]);
    std::string asciiPeriod;
    for (wchar_t value : period)
    {
        if (value > 127) return usage();
        asciiPeriod.push_back(static_cast<char>(value));
    }
    return run(std::filesystem::path(argv[1]), asciiPeriod, argc == 4);
}
#else
int main(int argc, char* argv[])
{
    if (argc != 3 && argc != 4) return usage();
    if (argc == 4 && std::string(argv[3]) != "--report") return usage();
    return run(std::filesystem::u8path(argv[1]), argv[2], argc == 4);
}
#endif
