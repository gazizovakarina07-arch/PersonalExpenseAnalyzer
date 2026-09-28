# Минимальная версия CMake, необходимая для проекта.
cmake_minimum_required(VERSION 3.20)

# Название проекта и используемый язык программирования.
project(PersonalExpenseAnalyzer LANGUAGES CXX)

# Все цели проекта компилируются по стандарту C++17.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Статическая библиотека с предметной логикой приложения.
# Позднее сюда будут добавляться другие файлы domain и application.
add_library(expense_core STATIC
    src/domain/Category.cpp
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

# Отдельная исполняемая программа с тестами класса Category.
add_executable(category_tests
    tests/CategoryTests.cpp
)

# Тесты проверяют тот же предметный код, что и приложение.
target_link_libraries(category_tests
    PRIVATE
        expense_core
)

# Регистрация тестовой программы в CTest.
add_test(
    NAME category_tests
    COMMAND category_tests
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

    target_compile_options(category_tests
        PRIVATE
            /utf-8
    )
endif()
