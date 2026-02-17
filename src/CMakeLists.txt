add_library(
    axio 
    axio/axio.cpp
    axio/base/arch.hpp
    axio/base/compiler.hpp
    axio/base/macros.hpp
    axio/base/os.hpp
    axio/base/types.hpp
    axio/container/array.hpp
    axio/container/vector.hpp
)
add_library(axio::axio ALIAS axio)

target_enable_clang_format(axio)
target_enable_clang_tidy(axio)
target_export_api(axio AXIO_API)