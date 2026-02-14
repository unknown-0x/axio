add_library(
    axio 
    axio/base/arch.hpp
    axio/base/compiler.hpp
    axio/base/macros.hpp
    axio/base/os.hpp
    axio/base/types.hpp
    axio/container/array.hpp
    axio/container/vector.hpp
)
set_target_properties(axio PROPERTIES LINKER_LANGUAGE CXX)

target_enable_clang_format(axio)
target_enable_clang_tidy(axio)
target_export_api(axio AXIO_API)