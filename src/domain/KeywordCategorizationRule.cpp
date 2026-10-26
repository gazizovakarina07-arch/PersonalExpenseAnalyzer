#include "personal_expense/domain/KeywordCategorizationRule.h"

#include <stdexcept>  // std::invalid_argument
#include <utility>    // std::move

namespace personal_expense::domain
{

KeywordCategorizationRule::KeywordCategorizationRule(
    std::string keyword,
    Category category)
    : keyword_(std::move(keyword)),
      category_(std::move(category))
{
    // Запрещаем пустое ключевое слово и слово,
    // состоящее только из пробелов, табуляций или переносов строк.
    if (keyword_.find_first_not_of(" \t\n\r") == std::string::npos)
    {
        throw std::invalid_argument(
            "Ключевое слово правила не должно быть пустым");
    }
}

bool KeywordCategorizationRule::matches(
    const Transaction& transaction) const
{
    // Проверяем соответствие категории виду операции.
    const bool matchingType =
        (transaction.isIncome()
            && category_.type() == CategoryType::Income)
        ||
        (transaction.isExpense()
            && category_.type() == CategoryType::Expense);

    // Если тип не подходит, искать слово уже не нужно.
    if (!matchingType)
    {
        return false;
    }

    // Ищем ключевое слово внутри описания операции.
    // npos означает, что совпадение не найдено.
    return transaction.description().find(keyword_)
        != std::string::npos;
}

const Category& KeywordCategorizationRule::category() const noexcept
{
    return category_;
}

}
