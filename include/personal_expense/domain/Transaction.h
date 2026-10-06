#pragma once

#include "personal_expense/domain/Account.h"
#include "personal_expense/domain/Category.h"
#include "personal_expense/domain/Date.h"

#include <cstdint>
#include <optional>
#include <string>

namespace personal_expense::domain
{

class Transaction
{
public:
    Transaction(
        Date date,
        std::string description,
        std::int64_t amountCents,
        Account account,
        std::optional<Category> category = std::nullopt);

    const Date& date() const noexcept;
    const std::string& description() const noexcept;
    std::int64_t amountCents() const noexcept;
    const Account& account() const noexcept;
    const std::optional<Category>& category() const noexcept;

    bool isIncome() const noexcept;
    bool isExpense() const noexcept;
    bool hasCategory() const noexcept;

    void assignCategory(Category category);
    void clearCategory() noexcept;

private:
    void ensureCategoryMatchesAmount(
        const Category& category) const;

    Date date_;
    std::string description_;
    std::int64_t amountCents_;
    Account account_;
    std::optional<Category> category_;
};

}
