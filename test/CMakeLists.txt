add_executable(
    test_math
    main.cpp
)

add_test(NAME test_math COMMAND test_math)

target_link_libraries(test_math PRIVATE math)
target_link_libraries(test_math PRIVATE simpletest::main)
target_include_directories(test_math PRIVATE ${PROJECT_ROOT}/src)

target_import_api(test_math math)
target_enable_clang_format(test_math)
target_enable_clang_tidy(test_math)
target_compiler_warnings(test_math)