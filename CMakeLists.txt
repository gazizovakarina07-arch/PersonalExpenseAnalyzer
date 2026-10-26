# Минимальная версия CMake, необходимая для проекта.
cmake_minimum_required(VERSION 3.20)

# Название проекта и используемый язык программирования.
project(PersonalExpenseAnalyzer LANGUAGES CXX)

# Все цели проекта компилируются по стандарту C++17.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Статическая библиотека с предметной логикой приложения.
#
# Здесь перечисляются все файлы .cpp,
# из которых должна состоять библиотека expense_core.
add_library(expense_core STATIC
    src/domain/Account.cpp
    src/domain/Category.cpp
    src/domain/Date.cpp
    src/domain/Transaction.cpp
    src/domain/KeywordCategorizationRule.cpp
    src/domain/ExactDescriptionRule.cpp
    src/application/TransactionCategorizer.cpp
    src/application/ReportGenerator.cpp
    src/infrastructure/CsvStatementImporter.cpp
)
# Папка с открытыми заголовочными файлами библиотеки.
# PUBLIC передаёт этот путь приложению и тестам,
# которые подключают expense_core.
target_include_directories(expense_core
    PUBLIC
        include
)

# Основное консольное приложение.
add_executable(PersonalExpenseAnalyzer
    src/presentation/main.cpp
    src/presentation/ReportPrinter.cpp
    src/presentation/ConsoleMenu.cpp
)

# Приложение использует предметную библиотеку expense_core.
target_link_libraries(PersonalExpenseAnalyzer
    PRIVATE
        expense_core
)

# Включение поддержки автоматических тестов CTest.
enable_testing()

# Отдельная исполняемая программа с тестами класса Account.
add_executable(account_tests
    tests/AccountTests.cpp
)

target_link_libraries(account_tests
    PRIVATE
        expense_core
)

add_test(
    NAME account_tests
    COMMAND account_tests
)

# Отдельная исполняемая программа с тестами класса Category.
add_executable(category_tests
    tests/CategoryTests.cpp
)

target_link_libraries(category_tests
    PRIVATE
        expense_core
)

add_test(
    NAME category_tests
    COMMAND category_tests
)

# Отдельная исполняемая программа с тестами класса Date.
add_executable(date_tests
    tests/DateTests.cpp
)

target_link_libraries(date_tests
    PRIVATE
        expense_core
)

add_test(
    NAME date_tests
    COMMAND date_tests
)

# Отдельная исполняемая программа с тестами класса Transaction.
add_executable(transaction_tests
    tests/TransactionTests.cpp
)

# Тесты Transaction используют классы из expense_core:
# Transaction, Date, Account и Category.
target_link_libraries(transaction_tests
    PRIVATE
        expense_core
)

# Регистрируем программу transaction_tests в системе CTest.
add_test(
    NAME transaction_tests
    COMMAND transaction_tests
)

# Компилятору Microsoft явно указывается кодировка UTF-8.
# Настройка задаётся отдельно для каждой компилируемой цели.
# Создаём программу проверки правила по ключевому слову.
add_executable(keyword_rule_tests
    tests/KeywordCategorizationRuleTests.cpp
)

# Подключаем реализацию предметных классов и правила.
target_link_libraries(keyword_rule_tests
    PRIVATE
        expense_core
)

# Позволяем CTest запускать эту программу.
add_test(
    NAME keyword_rule_tests
    COMMAND keyword_rule_tests
)
# Создаём программу проверки точного совпадения описания.
add_executable(exact_rule_tests
    tests/ExactDescriptionRuleTests.cpp
)

# Подключаем библиотеку с реализацией правила.
target_link_libraries(exact_rule_tests
    PRIVATE
        expense_core
)

# Регистрируем тест в CTest.
add_test(
    NAME exact_rule_tests
    COMMAND exact_rule_tests
)
# Проверка совместной работы обработчика и разных видов правил.
add_executable(categorizer_tests tests/TransactionCategorizerTests.cpp)
target_link_libraries(categorizer_tests PRIVATE expense_core)
add_test(NAME categorizer_tests COMMAND categorizer_tests)

add_executable(csv_import_tests tests/CsvStatementImporterTests.cpp)
target_link_libraries(csv_import_tests PRIVATE expense_core)
# Абсолютный путь не зависит от текущей рабочей папки CTest.
add_test(NAME csv_import_tests COMMAND csv_import_tests
    "${CMAKE_CURRENT_SOURCE_DIR}/data/demo_statement.csv")

add_executable(report_tests tests/ReportTests.cpp src/presentation/ReportPrinter.cpp)
target_link_libraries(report_tests PRIVATE expense_core)
add_test(NAME report_tests COMMAND report_tests)
add_test(NAME cli_tests COMMAND "${CMAKE_COMMAND}"
    "-DAPP=$<TARGET_FILE:PersonalExpenseAnalyzer>"
    "-DDEMO=${CMAKE_CURRENT_SOURCE_DIR}/data/demo_statement.csv"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/CliTests.cmake")

add_executable(menu_tests tests/ConsoleMenuTests.cpp
    src/presentation/ConsoleMenu.cpp src/presentation/ReportPrinter.cpp)
target_link_libraries(menu_tests PRIVATE expense_core)
add_test(NAME menu_tests COMMAND menu_tests)

if(MSVC)
    target_compile_options(menu_tests PRIVATE /utf-8)
    target_compile_options(report_tests PRIVATE /utf-8)
    target_compile_options(csv_import_tests PRIVATE /utf-8)
    target_compile_options(categorizer_tests PRIVATE /utf-8)
    target_compile_options(expense_core
        PRIVATE
            /utf-8
    )

    target_compile_options(PersonalExpenseAnalyzer
        PRIVATE
            /utf-8
    )

    target_compile_options(account_tests
        PRIVATE
            /utf-8
    )

    target_compile_options(category_tests
        PRIVATE
            /utf-8
    )

    target_compile_options(date_tests
        PRIVATE
            /utf-8
    )

    target_compile_options(transaction_tests
        PRIVATE
            /utf-8
    )
    # Русские строки нового теста компилируются как UTF-8.
    target_compile_options(keyword_rule_tests
        PRIVATE
            /utf-8
    )
    # Поддержка русского текста в исходном файле теста.
    target_compile_options(exact_rule_tests
        PRIVATE
            /utf-8
    )
endif()
