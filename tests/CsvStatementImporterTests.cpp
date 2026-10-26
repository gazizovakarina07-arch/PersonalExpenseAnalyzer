#include "personal_expense/infrastructure/CsvStatementImporter.h"
#include "personal_expense/application/TransactionCategorizer.h"
#include "personal_expense/domain/ExactDescriptionRule.h"
#include "personal_expense/domain/KeywordCategorizationRule.h"

#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace
{
using namespace personal_expense;
using namespace personal_expense::domain;
constexpr const char* header = "date;description;amount;account\n";

void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

application::ImportResult read(const std::string& data)
{
    std::istringstream input(data);
    return infrastructure::CsvStatementImporter().importStream(input);
}

void checkRows()
{
    // BOM, CRLF, разделитель внутри кавычек и удвоенные кавычки.
    const auto quoted = read(std::string("\xEF\xBB\xBF")
        + "date;description;amount;account\r\n"
        + "2024-02-29;\"Магазин; \"\"Сок\"\"\";-0.01;\"Основная карта\"\r\n");
    require(quoted.errors.empty() && quoted.transactions.size() == 1, "quoted CSV failed");
    require(quoted.transactions[0].description() == "Магазин; \"Сок\""
        && quoted.transactions[0].amountCents() == -1
        && quoted.transactions[0].date().toIsoString() == "2024-02-29"
        && quoted.transactions[0].account().name() == "Основная карта"
        && !quoted.transactions[0].hasCategory(), "parsed values differ");

    const auto partial = read(std::string(header)
        + "2026-10-01;Пятерочка;-650.00;Карта\n"
        + "2026-02-31;Ошибка;-1.00;Карта\n"
        + "2026-10-02;   ;-1.00;Карта\n"
        + "2026-10-02;Покупка;-1.00;\n"
        + "2026-10-02;Покупка;0.00;Карта\n"
        + "2026-10-02;Покупка;-1.00;Карта;лишнее\n"
        + "2026-10-02;\"Нет конца;-1.00;Карта\n"
        + "2026-10-03;Зарплата;+45000.00;Карта"); // Нет завершающего переноса.
    require(partial.transactions.size() == 2 && partial.errors.size() == 6,
        "invalid rows prevented partial import");
    for (std::size_t i = 0; i < partial.errors.size(); ++i)
        require(partial.errors[i].line == i + 3 && !partial.errors[i].message.empty(),
            "incorrect error line or missing message");
    require(partial.transactions[1].amountCents() == 4500000, "positive sign failed");

    // Неверные форматы и переполнение не должны тихо обрезаться/округляться.
    for (const std::string amount : {"1", "1.0", "1.001", "1,00", "1e2", ".01",
            " 1.00", "1.00x", "1 000.00", "--1.00", "-0.00",
            "92233720368547758.08", "-92233720368547758.09"})
    {
        const auto result = read(std::string(header) + "2026-10-01;Тест;" + amount + ";Карта\n");
        require(result.transactions.empty() && result.errors.size() == 1,
            "invalid amount accepted");
    }
    const auto limits = read(std::string(header)
        + "2026-10-01;Максимум;92233720368547758.07;Карта\n"
        + "2026-10-01;Минимум;-92233720368547758.08;Карта\n");
    require(limits.errors.empty() && limits.transactions.size() == 2, "limits rejected");
    require(limits.transactions[0].amountCents() == std::numeric_limits<std::int64_t>::max()
        && limits.transactions[1].amountCents() == std::numeric_limits<std::int64_t>::min(),
        "limits parsed incorrectly");

    for (const std::string row : {"", "2026-1-01;Тест;1.00;Карта",
            "2026-10-01;Те\"ст;1.00;Карта", "2026-10-01;\"Тест\"x;1.00;Карта",
            "2026-10-01;Тест;1.00"})
    {
        const auto result = read(std::string(header) + row + '\n');
        require(result.errors.size() == 1 && result.transactions.empty(), "bad CSV row accepted");
    }
    const auto invalidUtf8 = read(std::string(header) + "2026-10-01;" + char(0xFF) + ";1.00;Карта\n");
    require(invalidUtf8.errors.size() == 1, "invalid UTF-8 accepted");
    const auto empty = read(header);
    require(empty.transactions.empty() && empty.errors.empty(), "header-only file failed");
}

void checkFatalErrors()
{
    for (const std::string text : {"", "date,description,amount,account\n",
            "description;date;amount;account\n"})
    {
        bool rejected = false;
        try { read(text); }
        catch (const std::runtime_error&) { rejected = true; }
        require(rejected, "bad header accepted");
    }
    // Независящая от наличия конкретного файла ошибка открытия: пустой путь.
    bool rejected = false;
    try { infrastructure::CsvStatementImporter().importFile(std::filesystem::path{}); }
    catch (const std::runtime_error&) { rejected = true; }
    require(rejected, "unopenable file accepted");

    std::istringstream broken(header);
    broken.setstate(std::ios::badbit);
    rejected = false;
    try { infrastructure::CsvStatementImporter().importStream(broken); }
    catch (const std::runtime_error&) { rejected = true; }
    require(rejected, "broken stream accepted");
}

void checkFileAndCategorization(const std::filesystem::path& fixture)
{
    infrastructure::CsvStatementImporter importer;
    const application::IStatementImporter& interface = importer;
    auto result = interface.importFile(fixture);
    require(result.errors.empty() && result.transactions.size() == 4, "demo file import failed");
    application::TransactionCategorizer categorizer;
    categorizer.addRule(std::make_unique<KeywordCategorizationRule>(
        "Пятерочка", Category("Продукты", CategoryType::Expense)));
    categorizer.addRule(std::make_unique<ExactDescriptionRule>(
        "Зачисление зарплаты", Category("Зарплата", CategoryType::Income)));
    categorizer.addRule(std::make_unique<KeywordCategorizationRule>(
        "Аптека", Category("Здоровье", CategoryType::Expense)));
    int assigned = 0;
    for (auto& transaction : result.transactions)
        if (categorizer.categorize(transaction)) ++assigned;
    require(assigned == 3 && !result.transactions[1].hasCategory(), "integration categorization failed");
    require(result.transactions[0].category()->name() == "Продукты"
        && result.transactions[2].category()->name() == "Зарплата"
        && result.transactions[3].category()->name() == "Здоровье", "wrong integrated categories");
    std::cout << "Imported: 4; categorized: 3; without category: 1\n";
}
}

int main(int argc, char* argv[])
{
    try
    {
        require(argc == 2, "expected demo CSV path argument");
        checkRows();
        checkFatalErrors();
        checkFileAndCategorization(std::filesystem::u8path(argv[1]));
        std::cout << "PASSED: all CSV import tests\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAILED: " << error.what() << '\n';
        return 1;
    }
}
