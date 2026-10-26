#pragma once

#include "personal_expense/domain/ICategorizationRule.h"

#include <string>

namespace personal_expense::domain
{

// Правило ищет заданный текст внутри описания операции.
//
// Например, ключевое слово "Пятерочка" подходит
// для описаний "Пятерочка" и "Пятерочка №123".
//
// Сравнение учитывает регистр:
// "Пятерочка" и "пятерочка" считаются разным текстом.
class KeywordCategorizationRule : public ICategorizationRule
{
public:
    // Получает текст для поиска и назначаемую категорию.
    // Пустое или состоящее только из обычных пробельных
    // символов ключевое слово будет запрещено.
    KeywordCategorizationRule(
        std::string keyword,
        Category category);

    // Операция подходит, если:
    // 1. Описание содержит ключевое слово.
    // 2. Знак суммы соответствует типу категории.
    bool matches(
        const Transaction& transaction) const override;

    // Предоставляет категорию для назначения.
    const Category& category() const noexcept override;

private:
    // Текст, который ищем в описании.
    std::string keyword_;

    // Категория, которую предлагаем при совпадении.
    Category category_;
};

}
