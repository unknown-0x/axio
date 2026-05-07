#ifdef __cpp_char8_t
#define _TEXT(x) u8##x
#define TEXT(x) _TEXT(x)
#define CHAR char8_t
#define USE_CHAR8
#include "template/string_test.hpp"
#endif