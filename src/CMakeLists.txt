add_library(
    math 
    math/math.hpp
    math/math.cpp
)

target_enable_clang_format(math)
target_enable_clang_tidy(math)
target_export_api(math MATH_API)