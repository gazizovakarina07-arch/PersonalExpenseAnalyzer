// Подключаем объявление класса Transaction.
#include "personal_expense/domain/Transaction.h"

#include <stdexcept>  // std::invalid_argument
#include <utility>    // std::move

namespace personal_expense::domain
{

// Конструктор получает все данные новой операции.
Transaction::Transaction(
    Date date,
    std::string description,
    std::int64_t amountCents,
    Account account,
    std::optional<Category> category)
    // Переносим полученные значения в поля создаваемого объекта.
    : date_(std::move(date)),
      description_(std::move(description)),
      amountCents_(amountCents),
      account_(std::move(account)),
      category_(std::move(category))
{
    // find_first_not_of ищет первый символ, который не входит
    // в строку " \t\n\r". Если такого символа нет, метод
    // возвращает std::string::npos.
    if (description_.find_first_not_of(" \t\n\r")
        == std::string::npos)
    {
        throw std::invalid_argument(
            "Описание операции не должно быть пустым");
    }

    // Нулевая сумма не является ни доходом, ни расходом,
    // поэтому такую транзакцию создавать запрещаем.
    if (amountCents_ == 0)
    {
        throw std::invalid_argument(
            "Сумма операции не должна быть равна нулю");
    }

    // category_ является std::optional.
    // В условии if он превращается в true, если категория существует.
    if (category_)
    {
        // Звёздочка извлекает объект Category из optional.
        ensureCategoryMatchesAmount(*category_);
    }
}

// Возвращаем дату операции.
// Ссылка позволяет не создавать копию Date.
const Date& Transaction::date() const noexcept
{
    return date_;
}

// Возвращаем описание без копирования строки.
const std::string& Transaction::description() const noexcept
{
    return description_;
}

// Возвращаем сумму в копейках.
std::int64_t Transaction::amountCents() const noexcept
{
    return amountCents_;
}

// Возвращаем счёт без создания его копии.
const Account& Transaction::account() const noexcept
{
    return account_;
}

// Возвращаем optional, содержащий категорию или её отсутствие.
const std::optional<Category>& Transaction::category() const noexcept
{
    return category_;
}

// Положительная сумма означает доход.
bool Transaction::isIncome() const noexcept
{
    return amountCents_ > 0;
}

// Отрицательная сумма означает расход.
bool Transaction::isExpense() const noexcept
{
    return amountCents_ < 0;
}

// Проверяем, находится ли внутри optional категория.
bool Transaction::hasCategory() const noexcept
{
    return category_.has_value();
}

// Назначаем транзакции новую категорию.
void Transaction::assignCategory(Category category)
{
    // Сначала проверяем категорию.
    // Если проверка выбросит исключение, старая категория сохранится.
    ensureCategoryMatchesAmount(category);

    // После успешной проверки помещаем категорию в optional.
    category_ = std::move(category);
}

// Удаляем категорию из optional.
void Transaction::clearCategory() noexcept
{
    category_.reset();
}

// Проверяем, соответствует ли тип категории знаку суммы.
void Transaction::ensureCategoryMatchesAmount(
    const Category& category) const
{
    // Расход не может иметь категорию дохода.
    const bool expenseHasIncomeCategory =
        isExpense()
        && category.type() == CategoryType::Income;

    // Доход не может иметь категорию расхода.
    const bool incomeHasExpenseCategory =
        isIncome()
        && category.type() == CategoryType::Expense;

    if (expenseHasIncomeCategory || incomeHasExpenseCategory)
    {
        throw std::invalid_argument(
            "Тип категории не соответствует знаку суммы");
    }
}

}
