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

**Axio** is a utility library built using C++17 and above. It provides data structures, string utilities, and flexible type traits.

## Features

- **100% Header-Only:** No pre-compilation required. Just include the headers and you are good to go.
- **Zero Dependencies:** 100% self-contained. Clean and ultra-lightweight.
- **Cross-Platform:** Full support for Windows, Linux, and macOS. Seamlessly compiles with GCC, Clang, and MSVC.

---

## Examples

```c++
#include <axio/string/string.hpp>
#include <iostream>

int main() {
  axio::String msg = "Hello world!";

  if (msg.StartsWith("Hello")) {
    std::cout << msg.CStr() << std::endl;
  }
}

// Output:
// Hello world!
```

```c++
#include <axio/container/tuple.hpp>
#include <axio/string/string_utils.hpp>
#include <iostream>

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
  std::cout << concat_res.CStr() << std::endl;

  // Output: string - 42 - {Alice, 100pts}
  std::cout << join_res.CStr() << std::endl;
}
```

```c++
#include <axio/container/vector.hpp>
#include <axio/string/string_utils.hpp>
#include <iostream>

int main() {
  auto v = axio::Split("aa,bb,  cc , dd,ee  ,ff,gg", ',') | axio::Drop(2) |
           axio::Take(3) | axio::Trim |
           axio::To<axio::Vector<std::string_view>>();
  auto s = axio::StringJoin(v, "-");

  // Output: cc-dd-ee
  std::cout << s.CStr() << std::endl;
}
```

```c++
#include <axio/base/type_traits.hpp>
#include <axio/container/tuple.hpp>
#include <iostream>
#include <vector>

static_assert(
    axio::IsSpecializationOf<std::vector<int>, std::vector>::value,
    "");

static_assert(
    axio::IsSpecializationOf<axio::Tuple<int, double, int>, axio::Tuple>::value,
    "");

template <typename T>
using HasPushBack = decltype(std::declval<T>().push_back(
    std::declval<typename T::value_type>()));

template <typename C, typename V>
void TryPushBack(C& container, V&& value) {
  if constexpr (axio::IsDetected<HasPushBack, C>::value) {
    container.push_back(std::forward<V>(value));
    std::cout << "Ok.\n";
  } else {
    std::cout << "No push_back available!\n";
  }
}

int main() {
  std::vector<int> vec;
  std::array<int, 3> arr;

  TryPushBack(vec, 10);  // Output: Ok.
  TryPushBack(arr, 20);  // Output: No push_back available!
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
