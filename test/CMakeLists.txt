function (axio_test test_name)
    add_executable(
        ${test_name}
        ${ARGN}
    )
    add_test(NAME ${test_name} COMMAND ${test_name})

    target_link_libraries(
        ${test_name} simpletest::main axio::axio
    )
    target_include_directories(${test_name} PRIVATE ${PROJECT_ROOT}/src/)
    target_import_api(${test_name} axio::axio)
    target_enable_clang_format(${test_name})
    target_enable_clang_tidy(${test_name})
    target_compiler_warnings(${test_name})
endfunction()

axio_test(
    base_test 
    base/arch.cpp
    base/compiler.cpp
    base/macros.cpp
    base/os.cpp
    base/type_traits.cpp
    base/types.cpp
)

axio_test(
    container_test
    container/compressed_tuple.cpp
)

axio_test(
    memory_test 
    memory/allocator.cpp
)

axio_test(
    utility_test 
    utility/forward.cpp
    utility/move.cpp
)