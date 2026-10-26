#pragma once

#include "personal_expense/domain/Transaction.h"

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace personal_expense::application
{

// Номер физической строки файла: заголовок занимает строку 1.
struct ImportError
{
    std::size_t line;
    std::string message;
};

// Частичный успех: корректные операции и объяснения отклонённых строк.
struct ImportResult
{
    std::vector<domain::Transaction> transactions;
    std::vector<ImportError> errors;
};

class IStatementImporter
{
public:
    virtual ~IStatementImporter() = default;

    // Ошибки файла/заголовка выбрасывают исключение.
    // Ошибки отдельных операций возвращаются в ImportResult.
    virtual ImportResult importFile(const std::filesystem::path& path) const = 0;
};

}
