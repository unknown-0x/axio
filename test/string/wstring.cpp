#define _TEXT(x) L##x
#define TEXT(x) _TEXT(x)
#define CHAR wchar_t
#define USE_WCHAR
#include "template/string_test.hpp"