cmake_minimum_required(VERSION 3.20)

project(PersonalExpenseAnalyzer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(PersonalExpenseAnalyzer
    src/presentation/main.cpp
    src/domain/Category.cpp
)

target_include_directories(PersonalExpenseAnalyzer
    PRIVATE
        include
)

if(MSVC)
    target_compile_options(PersonalExpenseAnalyzer
        PRIVATE
            /utf-8
    )
endif()
