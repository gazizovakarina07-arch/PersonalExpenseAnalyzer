#include "personal_expense/application/TransactionCategorizer.h"

#include <stdexcept>
#include <utility>

namespace personal_expense::application
{

void TransactionCategorizer::addRule(
    std::unique_ptr<domain::ICategorizationRule> rule)
{
    // nullptr не содержит объекта: вызывать его методы нельзя.
    if (!rule)
    {
        throw std::invalid_argument("Нельзя добавить пустое правило");
    }

    // unique_ptr не копируется. Переносим владение в конец списка.
    rules_.push_back(std::move(rule));
}

bool TransactionCategorizer::categorize(domain::Transaction& transaction) const
{
    // Сохраняем в том числе выбор, сделанный пользователем вручную.
    if (transaction.hasCategory())
    {
        return false;
    }

    // Перебираем умные указатели без копирования, в порядке добавления.
    for (const auto& rule : rules_)
    {
        // Виртуальный вызов выбирает реализацию конкретного правила.
        if (rule->matches(transaction))
        {
            // Transaction дополнительно проверяет корректность категории.
            transaction.assignCategory(rule->category());
            return true; // Остальные правила уже не проверяются.
        }
    }

    // Операция остаётся без категории, но не удаляется и не теряется.
    return false;
}

}
