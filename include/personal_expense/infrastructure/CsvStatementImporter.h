#pragma once

#include "personal_expense/application/IStatementImporter.h"

#include <istream>

namespace personal_expense::infrastructure
{

class CsvStatementImporter : public application::IStatementImporter
{
public:
    application::ImportResult importFile(
        const std::filesystem::path& path) const override;

    // Тот же разбор для открытого потока, в том числе строки в тестах.
    application::ImportResult importStream(std::istream& input) const;
};

}
