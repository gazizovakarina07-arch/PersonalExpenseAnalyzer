#include "personal_expense/presentation/ConsoleMenu.h"
#include "personal_expense/presentation/ReportPrinter.h"
#include "personal_expense/domain/ExactDescriptionRule.h"
#include "personal_expense/domain/KeywordCategorizationRule.h"
#include <charconv>
#include <memory>
#include <stdexcept>
#include <utility>

namespace personal_expense::presentation
{
namespace
{
// getline читает всю строку, включая пробелы в русских названиях.
// false означает EOF: пользователь закрыл ввод, нельзя бесконечно повторять меню.
bool prompt(std::istream& input, std::ostream& output, const char* message, std::string& value)
{
    output << message << std::flush;
    if (!std::getline(input, value)) return false;
    if (!value.empty() && value.back() == '\r') value.pop_back();
    if (value == ":отмена") throw std::invalid_argument("Действие отменено");
    return true;
}
std::size_t number(const std::string& text)
{
    std::size_t value = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size())
        throw std::invalid_argument("Введите целый номер без дополнительных символов");
    return value;
}
}

ConsoleMenu::ConsoleMenu(std::vector<domain::Transaction>& transactions, int year, int month)
    : transactions_(transactions), year_(year), month_(month)
{
    const domain::Date valid(year, month, 1);
    addRule(false, "Пятерочка", domain::Category("Продукты", domain::CategoryType::Expense));
    addRule(false, "Аптека", domain::Category("Здоровье", domain::CategoryType::Expense));
    addRule(true, "Зачисление зарплаты", domain::Category("Зарплата", domain::CategoryType::Income));
}

void ConsoleMenu::addRule(bool exact, const std::string& text, domain::Category category)
{
    std::unique_ptr<domain::ICategorizationRule> rule;
    if (exact) rule = std::make_unique<domain::ExactDescriptionRule>(text, category);
    else rule = std::make_unique<domain::KeywordCategorizationRule>(text, category);
    // Сначала проверены все настройки. Если добавление правила не удалось,
    // откатываем описание, чтобы список отображал реальный порядок.
    descriptions_.push_back({exact, text, std::move(category)});
    try { categorizer_.addRule(std::move(rule)); }
    catch (...) { descriptions_.pop_back(); throw; }
}

void ConsoleMenu::applyInitialRules()
{
    for (auto& transaction : transactions_) categorizer_.categorize(transaction);
}

std::vector<std::size_t> ConsoleMenu::selectedIndices() const
{
    std::vector<std::size_t> indices;
    for (std::size_t i = 0; i < transactions_.size(); ++i)
        if (transactions_[i].date().year() == year_ && transactions_[i].date().month() == month_)
            indices.push_back(i);
    return indices;
}

void ConsoleMenu::showTransactions(std::ostream& output) const
{
    const auto indices = selectedIndices();
    if (indices.empty()) output << "В выбранном месяце нет операций.\n";
    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        const auto& transaction = transactions_[indices[i]];
        output << i + 1 << " | " << transaction.date().toIsoString()
            << " | " << transaction.description() << " | "
            << formatMoney(transaction.amountCents()) << " ₽ | "
            << transaction.account().name() << " | "
            << (transaction.hasCategory() ? transaction.category()->name() : "Без категории") << '\n';
    }
}

bool ConsoleMenu::assignCategory(std::istream& input, std::ostream& output)
{
    showTransactions(output);
    const auto indices = selectedIndices();
    if (indices.empty()) return true;
    std::string value;
    if (!prompt(input, output, "Номер операции: ", value)) return false;
    const auto selected = number(value);
    if (selected == 0 || selected > indices.size())
        throw std::invalid_argument("Нет операции с таким номером в выбранном месяце");
    auto& transaction = transactions_[indices[selected - 1]];
    output << "Тип категории: " << (transaction.isIncome() ? "доход" : "расход")
        << " (определён по знаку суммы).\n";
    if (!prompt(input, output, "Название категории: ", value)) return false;
    transaction.assignCategory(domain::Category(value, transaction.isIncome()
        ? domain::CategoryType::Income : domain::CategoryType::Expense));
    output << "Категория назначена только этой операции. Правило не создавалось.\n";
    return true;
}

bool ConsoleMenu::createRule(std::istream& input, std::ostream& output)
{
    std::string kind, text, name, type;
    if (!prompt(input, output, "Вид: 1 — содержит текст, 2 — точное описание: ", kind)) return false;
    if (kind != "1" && kind != "2") throw std::invalid_argument("Выберите вид правила 1 или 2");
    if (!prompt(input, output, "Текст для поиска (регистр учитывается): ", text)) return false;
    if (!prompt(input, output, "Название категории: ", name)) return false;
    if (!prompt(input, output, "Тип: 1 — расход, 2 — доход: ", type)) return false;
    if (type != "1" && type != "2") throw std::invalid_argument("Выберите тип 1 или 2");
    addRule(kind == "2", text, domain::Category(name, type == "1"
        ? domain::CategoryType::Expense : domain::CategoryType::Income));
    output << "Правило добавлено в конец списка. Для применения выберите пункт 5.\n";
    return true;
}

void ConsoleMenu::showRules(std::ostream& output) const
{
    output << "Первое совпадение побеждает. Первые три правила — встроенные.\n";
    for (std::size_t i = 0; i < descriptions_.size(); ++i)
    {
        const auto& rule = descriptions_[i];
        output << i + 1 << " | " << (rule.exact ? "Точно" : "Содержит")
            << " «" << rule.text << "» → " << rule.category.name()
            << " | " << (rule.category.type() == domain::CategoryType::Income ? "доход" : "расход") << '\n';
    }
}

void ConsoleMenu::applyRules(std::ostream& output)
{
    std::size_t assigned = 0, unknown = 0;
    for (auto index : selectedIndices())
    {
        if (categorizer_.categorize(transactions_[index])) ++assigned;
        if (!transactions_[index].hasCategory()) ++unknown;
    }
    output << "В выбранном месяце категории назначены: " << assigned
        << "\nОстались без категории: " << unknown << '\n';
}

void ConsoleMenu::run(std::istream& input, std::ostream& output)
{
    output << "\nИзменения хранятся только до закрытия программы. SQLite пока не подключён.\n"
        << "Исходный CSV не изменяется. Для отмены действия введите :отмена.\n";
    bool running = true;
    while (running)
    {
        output << "\n1 — Показать операции выбранного месяца\n"
            << "2 — Назначить или изменить категорию операции\n"
            << "3 — Создать правило категоризации\n4 — Показать правила\n"
            << "5 — Применить правила к операциям без категории выбранного месяца\n"
            << "6 — Показать месячный отчёт\n0 — Завершить работу\n";
        std::string choice;
        try
        {
            if (!prompt(input, output, "Выбор: ", choice)) break;
            if (choice == "0") break;
            if (choice == "1") showTransactions(output);
            else if (choice == "2") running = assignCategory(input, output);
            else if (choice == "3") running = createRule(input, output);
            else if (choice == "4") showRules(output);
            else if (choice == "5") applyRules(output);
            else if (choice == "6") printReport(output,
                application::ReportGenerator().generate(transactions_, year_, month_));
            else output << "Неизвестный пункт меню. Выберите число от 0 до 6.\n";
        }
        catch (const std::invalid_argument& error)
        { output << "Сообщение: " << error.what() << ". Повторите действие через меню.\n"; }
        catch (const std::overflow_error& error)
        { output << "Ошибка отчёта: " << error.what() << '\n'; }
    }
    output << "Сеанс завершён. Изменения категорий и правил не сохранены между запусками.\n";
}
}
