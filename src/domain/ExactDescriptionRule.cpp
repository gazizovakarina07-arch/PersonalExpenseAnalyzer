#include "personal_expense/domain/ExactDescriptionRule.h"

#include <stdexcept>  // std::invalid_argument
#include <utility>    // std::move

namespace personal_expense::domain
{

ExactDescriptionRule::ExactDescriptionRule(
    std::string description,
    Category category)
    : description_(std::move(description)),
      category_(std::move(category))
{
    // Ожидаемое описание не должно быть пустым
    // или состоять только из обычных пробельных символов.
    if (description_.find_first_not_of(" \t\n\r")
        == std::string::npos)
    {
        throw std::invalid_argument(
            "Описание для правила не должно быть пустым");
    }
}

bool ExactDescriptionRule::matches(
    const Transaction& transaction) const
{
    // Доходная категория подходит только доходам,
    // расходная — только расходам.
    const bool matchingType =
        (transaction.isIncome()
            && category_.type() == CategoryType::Income)
        ||
        (transaction.isExpense()
            && category_.type() == CategoryType::Expense);

    if (!matchingType)
    {
        return false;
    }

    // Сравниваем описание целиком, включая регистр и пробелы.
    return transaction.description() == description_;
}

const Category& ExactDescriptionRule::category() const noexcept
{
    return category_;
}

}
