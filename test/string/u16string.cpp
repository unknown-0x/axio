#define _TEXT(x) u##x
#define TEXT(x) _TEXT(x)
#define CHAR char16_t
#define USE_CHAR16
#include "template/string_test.hpp"