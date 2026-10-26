#include "personal_expense/infrastructure/CsvStatementImporter.h"

#include <cstdint>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <utility>

namespace personal_expense::infrastructure
{
namespace
{

// Проверяем UTF-8: запрещаем обрезанные последовательности, суррогаты
// и неканонические кодировки. Перекодирование и изменение регистра не делаем.
bool isUtf8(const std::string& text)
{
    std::size_t i = 0;
    while (i < text.size())
    {
        const auto first = static_cast<unsigned char>(text[i++]);
        if (first < 0x80)
        {
            if (first == 0) return false;
            continue;
        }
        int following;
        std::uint32_t code;
        std::uint32_t minimum;
        if (first >= 0xC2 && first <= 0xDF)
        { following = 1; code = first & 0x1F; minimum = 0x80; }
        else if (first >= 0xE0 && first <= 0xEF)
        { following = 2; code = first & 0x0F; minimum = 0x800; }
        else if (first >= 0xF0 && first <= 0xF4)
        { following = 3; code = first & 0x07; minimum = 0x10000; }
        else return false;

        for (int j = 0; j < following; ++j)
        {
            if (i == text.size()) return false;
            const auto next = static_cast<unsigned char>(text[i++]);
            if ((next & 0xC0) != 0x80) return false;
            code = (code << 6) | (next & 0x3F);
        }
        if (code < minimum || code > 0x10FFFF
            || (code >= 0xD800 && code <= 0xDFFF)) return false;
    }
    return true;
}

// Одна физическая строка = одна операция. В кавычках разрешены ; и "".
std::vector<std::string> splitRow(const std::string& line)
{
    std::vector<std::string> fields;
    std::size_t i = 0;
    for (;;)
    {
        std::string field;
        if (i < line.size() && line[i] == '"')
        {
            ++i;
            bool closed = false;
            while (i < line.size())
            {
                const char character = line[i++];
                if (character != '"') field += character;
                else if (i < line.size() && line[i] == '"')
                { field += '"'; ++i; }
                else { closed = true; break; }
            }
            if (!closed)
                throw std::invalid_argument("Незакрытые кавычки; перенос внутри поля не поддерживается");
            if (i < line.size() && line[i] != ';')
                throw std::invalid_argument("После закрывающей кавычки ожидается разделитель ;");
        }
        else
        {
            while (i < line.size() && line[i] != ';')
            {
                if (line[i] == '"')
                    throw std::invalid_argument("Кавычка внутри поля без внешних кавычек");
                field += line[i++];
            }
        }
        fields.push_back(std::move(field));
        if (i == line.size()) break;
        ++i; // Пропускаем разделитель; сохраняем и пустое последнее поле.
    }
    if (fields.size() != 4)
        throw std::invalid_argument("Ожидается ровно 4 столбца");
    return fields;
}

bool isDigit(char value) { return value >= '0' && value <= '9'; }

domain::Date parseDate(const std::string& text)
{
    if (text.size() != 10 || text[4] != '-' || text[7] != '-')
        throw std::invalid_argument("Дата должна иметь формат YYYY-MM-DD");
    for (std::size_t i = 0; i < text.size(); ++i)
        if (i != 4 && i != 7 && !isDigit(text[i]))
            throw std::invalid_argument("Дата должна содержать цифры в формате YYYY-MM-DD");
    // Календарную корректность проверяет уже существующий класс Date.
    return domain::Date(std::stoi(text.substr(0, 4)),
        std::stoi(text.substr(5, 2)), std::stoi(text.substr(8, 2)));
}

std::int64_t parseAmount(const std::string& text)
{
    std::size_t start = 0;
    const bool negative = !text.empty() && text[0] == '-';
    if (!text.empty() && (text[0] == '-' || text[0] == '+')) start = 1;
    const auto dot = text.find('.', start);
    if (dot == std::string::npos || dot == start || text.size() - dot != 3)
        throw std::invalid_argument("Сумма должна иметь формат -650.00 или 650.00");

    // Собираем копейки из цифр без double. Проверяем переполнение ДО умножения.
    const auto maximum = static_cast<std::uint64_t>(
        std::numeric_limits<std::int64_t>::max());
    const std::uint64_t limit = negative ? maximum + 1 : maximum;
    std::uint64_t cents = 0;
    for (std::size_t i = start; i < text.size(); ++i)
    {
        if (i == dot) continue;
        if (!isDigit(text[i]))
            throw std::invalid_argument("В сумме допустимы только цифры, знак и десятичная точка");
        const auto digit = static_cast<unsigned>(text[i] - '0');
        if (cents > (limit - digit) / 10)
            throw std::invalid_argument("Сумма выходит за диапазон int64_t в копейках");
        cents = cents * 10 + digit;
    }
    // Минимальное отрицательное значение нельзя получать отрицанием int64_t max+1.
    if (negative && cents == maximum + 1)
        return std::numeric_limits<std::int64_t>::min();
    const auto value = static_cast<std::int64_t>(cents);
    return negative ? -value : value;
}

void stripCarriageReturn(std::string& line)
{
    if (!line.empty() && line.back() == '\r') line.pop_back();
}
}

application::ImportResult CsvStatementImporter::importFile(
    const std::filesystem::path& path) const
{
    // filesystem::path поддерживает в том числе русские пути Windows.
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("Не удалось открыть CSV-файл");
    return importStream(input);
}

application::ImportResult CsvStatementImporter::importStream(std::istream& input) const
{
    std::string line;
    if (!std::getline(input, line))
        throw std::runtime_error("Не удалось прочитать заголовок CSV-файла");
    stripCarriageReturn(line);
    // Некоторые редакторы добавляют в начало UTF-8-файла три байта BOM.
    if (line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);
    if (line != "date;description;amount;account")
        throw std::runtime_error("Ожидается заголовок date;description;amount;account (UTF-8)");

    application::ImportResult result;
    std::size_t lineNumber = 1;
    while (std::getline(input, line))
    {
        ++lineNumber;
        stripCarriageReturn(line);
        try
        {
            if (!isUtf8(line))
                throw std::invalid_argument("Некорректный UTF-8 или нулевой символ");
            const auto fields = splitRow(line);
            const auto date = parseDate(fields[0]);
            const auto amount = parseAmount(fields[2]);
            const domain::Account account(fields[3]);
            // Конструктор Transaction проверяет описание и запрещает ноль.
            result.transactions.emplace_back(date, fields[1], amount, account);
        }
        catch (const std::invalid_argument& error)
        {
            result.errors.push_back({lineNumber, error.what()});
        }
    }
    // Ошибка устройства чтения не должна выглядеть как успешный частичный импорт.
    if (input.bad() || (input.fail() && !input.eof()))
        throw std::runtime_error("Ошибка чтения CSV-файла");
    return result;
}

}
