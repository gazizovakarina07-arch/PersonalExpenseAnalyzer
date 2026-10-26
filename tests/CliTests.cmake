# Проверяем реальную программу: результат, код выхода и восстановление после плохой строки.
function(check_run expected)
    execute_process(COMMAND "${APP}" ${ARGN} --report
        RESULT_VARIABLE code OUTPUT_VARIABLE output ERROR_VARIABLE error ENCODING UTF-8)
    if(NOT code EQUAL expected)
        message(FATAL_ERROR "Exit ${code}, expected ${expected}: ${output} ${error}")
    endif()
    set(last_output "${output}${error}" PARENT_SCOPE)
endfunction()
check_run(0 "${DEMO}" 2026-10)
foreach(fragment "Операций: 4" "45 000,00" "1 150,50" "43 849,50" "Без категории: 120,00")
    string(FIND "${last_output}" "${fragment}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Missing ${fragment}: ${last_output}")
    endif()
endforeach()
check_run(0 "${DEMO}" 2026-11)
if(NOT last_output MATCHES "Операций: 0")
    message(FATAL_ERROR "Month filter failed")
endif()
check_run(1 "${DEMO}" 2026-13)
check_run(1 "${DEMO}" 2026-1x)
check_run(1)
check_run(1 "${CMAKE_CURRENT_BINARY_DIR}/missing-dir/no-file.csv" 2026-10)
# Этот файл создаётся только внутри папки сборки, исходные данные не изменяются.
set(fixture "${CMAKE_CURRENT_BINARY_DIR}/проверка отчёта.csv")
file(WRITE "${fixture}" "date;description;amount;account\n2026-02-31;Ошибка;-1.00;Карта\n2026-10-01;ИП Иванов;-120.00;Карта\n")
check_run(2 "${fixture}" 2026-10)
if(NOT last_output MATCHES "Строка 2:" OR NOT last_output MATCHES "Операций: 1"
    OR NOT last_output MATCHES "Без категории: 120,00")
    message(FATAL_ERROR "Partial import failed: ${last_output}")
endif()

# Настоящий интерактивный процесс с UTF-8 командами вместо клавиатуры.
set(commands "${CMAKE_CURRENT_BINARY_DIR}/menu-input.txt")
file(WRITE "${commands}" "3\n2\nИП Иванов\nПродукты\n1\n4\n5\n6\n0\n")
execute_process(COMMAND "${APP}" "${DEMO}" 2026-10
    INPUT_FILE "${commands}" RESULT_VARIABLE code OUTPUT_VARIABLE menu_output
    ERROR_VARIABLE error ENCODING UTF-8 TIMEOUT 10)
if(NOT code EQUAL 0)
    message(FATAL_ERROR "Interactive run failed: ${code} ${error}")
endif()
foreach(fragment "4 | Точно «ИП Иванов»" "категории назначены: 1"
        "Продукты: 770,00" "Изменения хранятся только до закрытия")
    string(FIND "${menu_output}" "${fragment}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Missing interactive output ${fragment}: ${menu_output}")
    endif()
endforeach()
