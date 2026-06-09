<h1 align="center">AXIO</h1>

<p align="center">
  <strong>A modern, zero-dependency, header-only C++ utility library.</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17%2F20-blue.svg?style=flat-square" alt="C++ Standard">
  <img src="https://img.shields.io/badge/Header--Only-Yes-brightgreen.svg?style=flat-square" alt="Header Only">
  <img src="https://img.shields.io/badge/Dependencies-Zero-success.svg?style=flat-square" alt="Dependencies">
</p>

---

**Axio** is a utility library built using C++17 and above. It provides data structures, string utilities, flexible type traits, and more!!!

## Features

- **100% Header-Only:** No pre-compilation required. Just include the headers and you are good to go.
- **Zero Dependencies:** 100% self-contained. Clean and ultra-lightweight.
- **Cross-Platform:** Full support for Windows, Linux, and macOS. Seamlessly compiles with GCC, Clang, and MSVC.

---

## Examples

```c++
#include <axio/string/string.hpp>
#include <axio/utility/print.hpp>

int main() {
  axio::String msg = "Hello world!";

  if (msg.StartsWith("Hello")) {
    axio::Print(msg, ' ', 42);
  }
}

// Output:
// Hello world! 42
```

```c++
#include <axio/container/tuple.hpp>
#include <axio/string/string_utils.hpp>
#include <axio/utility/print.hpp>

struct Player {
  std::string name;
  int score;

  template <typename Output>
  friend void AxioRepr(Output& output, const Player& p) {
    axio::AppendToOutput(output, '{', p.name, ", ", p.score, "pts}");
  }
};

int main() {
  axio::Tuple<axio::String, int, Player> tuple{"string", 42, {"Alice", 100}};
  auto concat_res = axio::StringCat(tuple, " ", 42);
  auto join_res = axio::StringJoin(tuple, " - ");

  // Output: (string, 42, {Alice, 100pts}) 42
  axio::PrintLn(concat_res);
  // Output: string - 42 - {Alice, 100pts}
  axio::PrintLn(join_res);
}
```

```c++
#include <axio/container/vector.hpp>
#include <axio/string/string_utils.hpp>
#include <axio/utility/print.hpp>

int main() {
  auto v = axio::Split("aa,bb,  cc , dd,ee  ,ff,gg", ',') | axio::Drop(2) |
           axio::Take(3) | axio::Trim |
           axio::To<axio::Vector<std::string_view>>();
  auto s = axio::StringJoin(v, "-");

  // Output: cc-dd-ee
  axio::Print(s);
}
```

```c++
#include <array>
#include <axio/base/type_traits.hpp>
#include <axio/container/tuple.hpp>
#include <axio/utility/print.hpp>
#include <vector>

static_assert(axio::IsSpecializationOf_V<std::vector<int>, std::vector>, "");

static_assert(
    axio::IsSpecializationOf_V<axio::Tuple<int, double, int>, axio::Tuple>,
    "");

template <typename T>
using HasPushBack = decltype(std::declval<T>().push_back(
    std::declval<typename T::value_type>()));

template <typename C, typename V>
void TryPushBack(C& container, V&& value) {
  if constexpr (axio::IsDetected_V<HasPushBack, C>) {
    container.push_back(axio::Forward<V>(value));
    axio::PrintLn("Ok.");
  } else {
    axio::PrintLn("No push_back available!");
  }
}

int main() {
  std::vector<int> vec;
  std::array<int, 3> arr;

  TryPushBack(vec, 10);  // Output: Ok.
  TryPushBack(arr, 20);  // Output: No push_back available!
}
```

```c++
#include <axio/utility/defer.hpp>
#include <axio/utility/print.hpp>
#include <string_view>

void ProcessFile(std::string_view path) {
  AXIO_DEFER([&] { axio::PrintLn("Closing file: ", path); });

  axio::PrintLn("Opening file: ", path);

  const char* data = path.data();
  if (data) {
    return;
  }
}

int main() {
  ProcessFile("foo/document.txt");
  return 0;
}

// Output:
// Opening file: foo/document.txt
// Closing file: foo/document.txt
```

```c++
#include <axio/functional/small_function.hpp>
#include <axio/utility/print.hpp>
#include <string_view>

int Add(int x, int y) {
  return x + y;
}

struct LargeFunctor {
  int arr[10];

  LargeFunctor(int value) {
    for (int& x : arr) {
      x = value;
    }
  }

  int operator()(int extra) const {
    for (int x : arr) {
      extra += x;
    }
    return extra;
  }
};

int main() {
  int x = 1, y = 2;
  axio::SmallFunction<int(int, int)> f1 = Add;
  axio::SmallFunction<int(int, int)> f2 = [x, y](int a, int b) {
    return x + y + a + b;
  };
  axio::SmallFunction<int(int)> f3 = LargeFunctor(3);  // heap
  axio::SmallFunction<int(int), sizeof(LargeFunctor), alignof(LargeFunctor)>
      f4 = LargeFunctor{1};  // stack

  axio::PrintLn(f1(1, 2));  // 3
  axio::PrintLn(f2(1, 2));  // 6
  axio::PrintLn(f3(1));     // 31
  axio::PrintLn(f4(1));     // 11
  return 0;
}
```

```c++
#include <axio/string/string.hpp>
#include <axio/utility/print.hpp>
#include <axio/utility/result.hpp>

axio::Result<int, axio::String> Divide(int a, int b) {
  if (b == 0) {
    return axio::Error("Division by zero");
  }
  return axio::Ok(a / b);
}

void TryDivide(int a, int b) {
  auto result = Divide(a, b);

  if (result) {
    axio::PrintLn(a, '/', b, '=', *result);
  } else {
    axio::PrintLn("Error: ", result.GetError());
  }
}

int main() {
  TryDivide(10, 2);  // 10/2=5
  TryDivide(10, 0);  // Error: Division by zero
  return 0;
}
```

```c++
#include <axio/utility/print.hpp>
#include <axio/utility/result.hpp>
#include <string>

using Result = axio::Result<int, std::string>;

Result Parse(const std::string& s) {
  if (s.empty()) {
    return axio::Error("Empty string");
  }
  return axio::Ok(std::stoi(s));
}

Result Reciprocal(int x) {
  if (x == 0) {
    return axio::Error("Division by zero");
  }
  return axio::Ok(100 / x);
}

Result AddFive(int x) {
  return axio::Ok(x + 5);
}

void TryParse(const std::string& s) {
  auto result =
      Parse(s).Then(Reciprocal).Then(AddFive).Map([](int x) { return x * 2; });

  if (result) {
    axio::PrintLn(*result);
  } else {
    axio::PrintLn(result.GetError());
  }
}

int main() {
  TryParse("20");  // 20
  TryParse("0");   // Division by zero
  TryParse("");    // Empty string
  return 0;
}
```
---

## Integration

Since **Axio** is a header-only library, integrating it into your existing CMake project is straightforward.

Add the following to your `CMakeLists.txt` to automatically download and link the library at configure time:

```cmake
include(FetchContent)

FetchContent_Declare(
    axio
    GIT_REPOSITORY https://github.com/unknown-0x/axio.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(axio)

target_link_libraries(your_project PRIVATE axio::axio)
```

## Building & Testing

### Prerequisites

- **CMake** >= 3.15
- **Ninja** (required for `ninja` and `msvc` presets)
- A supported toolchain (GCC/Clang for Linux/macOS, MSVC/clang-cl for Windows)

| Platform / Toolchain      | Build Type             | Configure Preset                             | Build Preset                                 | Test Preset                                  |
| :------------------------ | :--------------------- | :------------------------------------------- | :------------------------------------------- | :------------------------------------------- |
| **Linux / macOS** (Ninja) | `Debug` <br> `Release` | `ninja-debug` <br> `ninja-release`           | `ninja-debug` <br> `ninja-release`           | `ninja-debug` <br> `ninja-release`           |
| **Windows** (MSVC)        | `Debug` <br> `Release` | `msvc-debug` <br> `msvc-release`             | `msvc-debug` <br> `msvc-release`             | `msvc-debug` <br> `msvc-release`             |
| **Windows** (clang-cl)    | `Debug` <br> `Release` | `msvc-clang-debug` <br> `msvc-clang-release` | `msvc-clang-debug` <br> `msvc-clang-release` | `msvc-clang-debug` <br> `msvc-clang-release` |

**Usage Example:**

```bash
# Configure Preset
cmake --preset ninja-release
# Build Preset
cmake --build --preset ninja-release
# Test Preset
ctest --preset ninja-release
```

## License

[License](https://github.com/unknown-0x/axio/blob/main/LICENSE)
