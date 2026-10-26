#pragma once

#include "personal_expense/domain/ICategorizationRule.h"
#include "personal_expense/domain/Transaction.h"

#include <memory>
#include <vector>

namespace personal_expense::application
{

// Организует применение правил, не зная их конкретных типов.
class TransactionCategorizer
{
public:
    // Передаёт владение правилом обработчику. Пустой указатель запрещён.
    void addRule(std::unique_ptr<domain::ICategorizationRule> rule);

    // true: этот вызов назначил категорию.
    // false: категория уже есть или подходящего правила нет.
    bool categorize(domain::Transaction& transaction) const;

private:
    // Порядок добавления определяет приоритет: первое совпадение побеждает.
    std::vector<std::unique_ptr<domain::ICategorizationRule>> rules_;
};

}
