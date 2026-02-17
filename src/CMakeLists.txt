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

if(BUILD_SHARED_LIBS AND MSVC)
  set_target_properties(axio
    PROPERTIES
      WINDOWS_EXPORT_ALL_SYMBOLS ON
  )
endif()

target_enable_clang_format(axio)
target_enable_clang_tidy(axio)
target_export_api(axio AXIO_API)