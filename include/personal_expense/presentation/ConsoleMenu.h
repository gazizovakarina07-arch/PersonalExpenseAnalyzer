#pragma once
#include "personal_expense/application/TransactionCategorizer.h"
#include <istream>
#include <ostream>
#include <string>
#include <vector>

namespace personal_expense::presentation
{
// Один сеанс работы. Ссылку на операции получаем от main; CSV не изменяем.
class ConsoleMenu
{
public:
    ConsoleMenu(std::vector<domain::Transaction>& transactions, int year, int month);
    void applyInitialRules();
    void run(std::istream& input, std::ostream& output);

private:
    struct RuleDescription
    {
        bool exact;
        std::string text;
        domain::Category category;
    };
    void addRule(bool exact, const std::string& text, domain::Category category);
    std::vector<std::size_t> selectedIndices() const;
    void showTransactions(std::ostream& output) const;
    bool assignCategory(std::istream& input, std::ostream& output);
    bool createRule(std::istream& input, std::ostream& output);
    void showRules(std::ostream& output) const;
    void applyRules(std::ostream& output);

    std::vector<domain::Transaction>& transactions_;
    int year_;
    int month_;
    application::TransactionCategorizer categorizer_;
    // Описания в том же порядке, что правила; добавление централизовано в addRule.
    std::vector<RuleDescription> descriptions_;
};
}
