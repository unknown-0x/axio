#define _TEXT(x) U##x
#define TEXT(x) _TEXT(x)
#define CHAR char32_t
#define USE_CHAR32
#include "template/string_test.hpp"