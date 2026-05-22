add_library(axio INTERFACE)
add_library(axio::axio ALIAS axio)

target_include_directories(axio INTERFACE ${PROJECT_ROOT}/src/)

file(GLOB_RECURSE AXIO_HEADERS "axio/*.hpp")
target_sources(axio INTERFACE ${AXIO_HEADERS})

if(BUILD_SHARED_LIBS AND MSVC)
  set_target_properties(axio
    PROPERTIES
      WINDOWS_EXPORT_ALL_SYMBOLS ON
  )
endif()

target_enable_clang_format(axio)
target_enable_clang_tidy(axio)