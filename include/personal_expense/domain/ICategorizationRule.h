#pragma once

#include "personal_expense/domain/Category.h"
#include "personal_expense/domain/Transaction.h"

namespace personal_expense::domain
{

// Общий интерфейс для всех правил категоризации.
class ICategorizationRule
{
public:
    // Позволяет корректно уничтожать конкретные правила
    // через указатель на общий интерфейс.
    virtual ~ICategorizationRule() = default;

    // Проверяет, подходит ли операция под условие правила.
    // Не изменяет переданную транзакцию.
    virtual bool matches(
        const Transaction& transaction) const = 0;

    // Возвращает категорию, которую предлагает правило.
    virtual const Category& category() const noexcept = 0;
};

}
