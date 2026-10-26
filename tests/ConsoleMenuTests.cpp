#include "personal_expense/presentation/ConsoleMenu.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace
{
using namespace personal_expense;
using namespace domain;
void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}
std::vector<Transaction> sample()
{
    return {
        Transaction(Date(2026, 9, 1), "ИП Иванов", -100, Account("Карта")),
        Transaction(Date(2026, 10, 1), "Пятерочка", -65000, Account("Карта")),
        Transaction(Date(2026, 10, 2), "ИП Иванов", -12000, Account("Карта")),
        Transaction(Date(2026, 10, 3), "ИП Иванов", -3000, Account("Карта")),
        Transaction(Date(2026, 10, 4), "Подарок", 50000, Account("Карта"))};
}
std::string session(presentation::ConsoleMenu& menu, const std::string& commands)
{
    std::istringstream input(commands);
    std::ostringstream output;
    menu.run(input, output);
    return output.str();
}
void check()
{
    auto values = sample();
    presentation::ConsoleMenu menu(values, 2026, 10);
    menu.applyInitialRules();
    auto text = session(menu, "1\n2\n2\nЕда\n6\n0\n");
    require(values[2].category()->name() == "Еда" && !values[3].hasCategory(),
        "manual choice affected wrong transaction");
    require(!values[0].hasCategory() && text.find("2026-09-01") == std::string::npos,
        "month scope broken");
    require(text.find("Еда: 120,00") != std::string::npos, "report not refreshed");

    // Добавление само по себе не назначает категории.
    text = session(menu, "3\n2\nИП Иванов\nПродукты\n1\n4\n0\n");
    require(!values[3].hasCategory() && text.find("4 | Точно «ИП Иванов»") != std::string::npos,
        "rule creation applied silently or order incorrect");
    text = session(menu, "5\n5\n0\n");
    require(values[3].category()->name() == "Продукты" && values[2].category()->name() == "Еда",
        "rule did not apply or manual category overwritten");
    require(!values[0].hasCategory() && text.find("категории назначены: 0") != std::string::npos,
        "repeat apply or month scope broken");
    text = session(menu, "3\n1\nПодар\nПодарки\n2\n5\n0\n");
    require(values[4].hasCategory() && values[4].category()->type() == CategoryType::Income,
        "income keyword rule failed");

    // Замена уже назначенной категории, тип вычисляется по операции.
    session(menu, "2\n1\nДом\n5\n0\n");
    require(values[1].category()->name() == "Дом", "existing category cannot be replaced");

    // Неверный ввод не завершает сеанс и не меняет категорию.
    text = session(menu, "9\n2\n0\n2\n999999999999999999999999\n2\n1x\n2\n1\n   \n"
        "3\n9\n3\n2\nИП Иванов\nЕда\n7\n2\n:отмена\n4\n0\n");
    require(values[1].category()->name() == "Дом" && text.find("Неизвестный пункт") != std::string::npos
        && text.find("Действие отменено") != std::string::npos, "invalid input recovery failed");
    require(text.find("6 |") == std::string::npos, "invalid rule was added");

    // EOF внутри действия должен завершить сеанс без частичного изменения.
    auto interrupted = sample();
    presentation::ConsoleMenu eofMenu(interrupted, 2026, 10);
    session(eofMenu, "2\n2\n");
    require(!interrupted[2].hasCategory(), "EOF changed category");
    session(eofMenu, "3\n2\nИП Иванов\nЕда\n");
    text = session(eofMenu, "4\n0\n");
    require(text.find("4 |") == std::string::npos, "EOF created partial rule");
    session(eofMenu, "");
    std::vector<Transaction> empty;
    presentation::ConsoleMenu emptyMenu(empty, 2026, 10);
    text = session(emptyMenu, "1\n2\n5\n6\n0\n");
    require(text.find("нет операций") != std::string::npos, "empty month failed");
}
}
int main()
{
    try { check(); std::cout << "PASSED: menu tests\n"; return 0; }
    catch (const std::exception& e) { std::cerr << "FAILED: " << e.what() << '\n'; return 1; }
}
