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
if(MSVC)
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
endif()