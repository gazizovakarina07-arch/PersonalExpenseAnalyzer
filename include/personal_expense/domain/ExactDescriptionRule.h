#pragma once

#include "personal_expense/domain/ICategorizationRule.h"

#include <string>

namespace personal_expense::domain
{

// Правило сравнивает описание операции целиком.
// Регистр, пробелы и остальные символы должны совпадать.
class ExactDescriptionRule : public ICategorizationRule
{
public:
    // Получает ожидаемое описание и предлагаемую категорию.
    ExactDescriptionRule(
        std::string description,
        Category category);

    // Проверяет полное совпадение описания
    // и соответствие типа категории знаку суммы.
    bool matches(
        const Transaction& transaction) const override;

    // Предоставляет категорию правила.
    const Category& category() const noexcept override;

private:
    // Описание, с которым сравниваем текст операции.
    std::string description_;

    // Категория для подходящей операции.
    Category category_;
};

}
