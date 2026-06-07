#include <simpletest/simpletest.hpp>

#include <axio/container/span.hpp>
#include <axio/container/vector.hpp>

#include <vector>

using axio::Array;
using axio::IsSame_V;
using axio::PtrDiffT;
using axio::SizeT;
using axio::Span;

static_assert(IsSame_V<Span<int>::ElementType, int>);
static_assert(IsSame_V<Span<int>::ValueType, int>);
static_assert(IsSame_V<Span<const int>::ElementType, const int>);
static_assert(IsSame_V<Span<const int>::ValueType, int>);
static_assert(IsSame_V<Span<int>::Pointer, int*>);
static_assert(IsSame_V<Span<int>::ConstPointer, const int*>);
static_assert(IsSame_V<Span<int>::Reference, int&>);
static_assert(IsSame_V<Span<int>::ConstReference, const int&>);
static_assert(IsSame_V<Span<int>::Iterator, int*>);
static_assert(IsSame_V<Span<int>::ConstIterator, const int*>);
static_assert(IsSame_V<Span<int>::SizeType, SizeT>);
static_assert(IsSame_V<Span<int>::DifferenceType, PtrDiffT>);

static_assert(Span<int>::kExtent == axio::kDynamicExtent);
static_assert(Span<int, 5>::kExtent == 5);
static_assert(Span<int, 0>::kExtent == 0);

static_assert(sizeof(Span<int>) == sizeof(int*) + sizeof(SizeT));
static_assert(sizeof(Span<int, 5>) == sizeof(int*));
static_assert(sizeof(Span<int, 0>) == sizeof(int*));

static_assert(axio::IsTriviallyCopyable_V<Span<int>>);
static_assert(axio::IsTriviallyCopyable_V<Span<int, 4>>);

static_assert(axio::IsNothrowCopyConstructible_V<Span<int>>);
static_assert(axio::IsNothrowCopyAssignable_V<Span<int>>);
static_assert(axio::IsNothrowDefaultConstructible_V<Span<int>>);
static_assert(axio::IsNothrowDefaultConstructible_V<Span<int, 0>>);

static_assert(axio::IsDefaultConstructible_V<Span<int>>);
static_assert(axio::IsDefaultConstructible_V<Span<int, 0>>);
static_assert(!axio::IsDefaultConstructible_V<Span<int, 3>>);
static_assert(!axio::IsDefaultConstructible_V<Span<int, 1>>);

static_assert(axio::IsConstructible_V<Span<const int>, Span<int>>);
static_assert(!axio::IsConstructible_V<Span<int>, Span<const int>>);

static_assert(axio::IsConstructible_V<Span<int, 3>, Span<int, 3>>);
static_assert(!axio::IsConstructible_V<Span<int, 3>, Span<int, 4>>);

namespace constexpr_tests {
static constexpr int arr1[] = {1, 2, 3, 4, 5};
static constexpr Array<int, 7> arr2{10, 20, 30, 40, 50, 60, 70};
static constexpr std::array<int, 7> arr3{10, 20, 30, 40, 50, 60, 70};

static constexpr Span<const int> s1(arr1, AXIO_ARRAY_SIZE(arr1));
static constexpr Span<const int> s2(arr2);
static constexpr Span<const int> s3(arr3);
static_assert(s1.Size() == 5);
static_assert(!s1.IsEmpty());
static_assert(s1.SizeBytes() == 5 * sizeof(int));
static_assert(s1.Front() == 1);
static_assert(s1.Back() == 5);
static_assert(s1[0] == 1);
static_assert(s1[1] == 2);
static_assert(s1[2] == 3);
static_assert(s1[3] == 4);
static_assert(s1[4] == 5);
static_assert(s1.Data() == arr1);

static_assert(s2.Data() == arr2.Data());
static_assert(s2.Size() == arr2.Size());
static_assert(s3.Data() == arr3.data());
static_assert(s3.Size() == arr3.size());

static constexpr Span<const int, 5> s4(arr1);
static_assert(s4.Size() == 5);
static_assert(s4[0] == 1);
static_assert(s4[1] == 2);
static_assert(s4[2] == 3);
static_assert(s4[3] == 4);
static_assert(s4[4] == 5);

static constexpr auto s5 = s2.First<2>();
static_assert(decltype(s5)::kExtent == 2);
static_assert(s5.Size() == 2);
static_assert(s5[0] == 10);
static_assert(s5[1] == 20);

static constexpr auto s6 = s2.Last<3>();
static_assert(decltype(s6)::kExtent == 3);
static_assert(s6.Size() == 3);
static_assert(s6[0] == 50);
static_assert(s6[1] == 60);
static_assert(s6[2] == 70);

static constexpr auto s7 = s2.Subspan<1, 3>();
static_assert(decltype(s7)::kExtent == 3);
static_assert(s7.Size() == 3);
static_assert(s7[0] == 20);
static_assert(s7[1] == 30);
static_assert(s7[2] == 40);

static constexpr auto s8 = s2.Subspan<2>();
static_assert(s8.Size() == 5);
static_assert(s8.Front() == 30);

static constexpr Span<const int, 7> s9(arr2);
static constexpr auto s10 = s9.Subspan<2>();
static_assert(decltype(s10)::kExtent == 5);
static_assert(s10.Size() == 5);
static_assert(s10.Front() == 30);

static constexpr Span<int, 0> empty;
static_assert(empty.Size() == 0);
static_assert(empty.IsEmpty());

static constexpr auto p = s9.Split<4>();
static constexpr auto head = p.first;
static constexpr auto tail = p.second;
static_assert(decltype(head)::kExtent == 4);
static_assert(decltype(tail)::kExtent == 3);
static_assert(head.Front() == 10);
static_assert(head.Back() == 40);
static_assert(tail.Front() == 50);
static_assert(tail.Back() == 70);
}  // namespace constexpr_tests

template <typename T>
void CheckSpan(Span<T> s, std::initializer_list<T> expected) {
  CHECK_EQ(s.Size(), expected.size());

  auto sb = s.begin();
  auto eb = expected.begin();

  for (SizeT i = 0; i < s.Size(); ++i) {
    CHECK_EQ(*sb, *eb);
    ++sb, ++eb;
  }
}

#define CHECK_SPAN(s, ...) \
  CheckSpan<decltype(s)::ElementType>(s, {__VA_ARGS__});

TEST_CASE(Span, DefaultConstructor) {
  Span<int> s1;
  CHECK_TRUE(s1.IsEmpty());
  CHECK_EQ(s1.Data(), nullptr);
  CHECK_EQ(s1.begin(), s1.end());

  Span<int, 0> s2;
  CHECK_TRUE(s2.IsEmpty());
  CHECK_EQ(s2.Data(), nullptr);
  CHECK_EQ(s2.begin(), s2.end());
}

TEST_CASE(Span, ConstructorPtrCount) {
  int arr[] = {1, 2, 3, 4, 5};
  Span<int> s1(arr, 5);
  CHECK_EQ(s1.Size(), 5);
  CHECK_EQ(s1.Data(), arr);
  CHECK_EQ(s1.Front(), 1);
  CHECK_EQ(s1.Back(), 5);

  Span<int, 5> s2(arr, 5);
  CHECK_EQ(s2.Size(), 5);
  CHECK_EQ(s2.Data(), arr);

  Span<int> s3(arr, SizeT(0));
  CHECK_EQ(s3.Size(), 0);
  CHECK_EQ(s3.Data(), arr);

  Span<const int> s4(arr, 3);
  CHECK_EQ(s4.Size(), 3);
  CHECK_EQ(s4.Front(), 1);
  CHECK_EQ(s4.Back(), 3);

  Span<int> s5(nullptr, SizeT(0));
  CHECK_EQ(s5.Size(), 0);
  CHECK_EQ(s5.Data(), nullptr);
}

TEST_CASE(Span, Constructor2Ptr) {
  int arr[] = {11, 22, 33, 44, 55, 66, 77};

  Span<int> s1(arr, arr + AXIO_ARRAY_SIZE(arr));
  CHECK_EQ(s1.Front(), 11);
  CHECK_EQ(s1.Back(), 77);
  CHECK_EQ(s1.Size(), 7);

  Span<int> s2(arr, arr);
  CHECK_EQ(s2.Size(), 0);
  CHECK_EQ(s2.Data(), arr);

  Span<int> s3(arr + 1, arr + 2);
  CHECK_EQ(s3.Size(), 1);
  CHECK_EQ(s3[0], 22);
}

TEST_CASE(Span, ConstructorCArray) {
  int arr[] = {1, 2, 3, 4, 5};
  Span<int> s1(arr);
  CHECK_EQ(s1.Size(), 5);
  CHECK_EQ(s1.Data(), arr);
  CHECK_SPAN(s1, 1, 2, 3, 4, 5);

  Span<int, 5> s2(arr);
  CHECK_EQ(s2.Size(), 5);
  CHECK_EQ(decltype(s2)::kExtent, 5);
  CHECK_SPAN(s2, 1, 2, 3, 4, 5);
  s2[0] = 4;
  CHECK_SPAN(s2, 4, 2, 3, 4, 5);
  CHECK_EQ(arr[0], 4);

  const int carr[] = {1, 2, 3};
  Span<const int> s3(carr);
  CHECK_EQ(s3.Size(), 3);
  CHECK_SPAN(s3, 1, 2, 3);
}

TEST_CASE(Span, ConstructorWithStdArray) {
  std::array<int, 4> a1{11, 22, 33, 44};

  Span<int> s1(a1);
  CHECK_SPAN(s1, 11, 22, 33, 44);
  CHECK_EQ(s1.Data(), a1.data());

  Span<int, 4> s2(a1);
  CHECK_SPAN(s2, 11, 22, 33, 44);
  CHECK_EQ(s1.Data(), a1.data());
  s2[0] = 55;
  CHECK_SPAN(s2, 55, 22, 33, 44);
  CHECK_EQ(a1[0], 55);

  std::array<int, 1> a2{111};
  Span<int> s3(a2);
  CHECK_SPAN(s3, 111);
}

TEST_CASE(Span, ConstructorWithConstStdArray) {
  const std::array<int, 3> ca{4, 5, 6};
  Span<const int> s1(ca);
  CHECK_SPAN(s1, 4, 5, 6);
  CHECK_EQ(s1.Data(), ca.data());

  Span<const int, 3> s2(ca);
  CHECK_SPAN(s2, 4, 5, 6);
  CHECK_EQ(s2.Data(), ca.data());
}

TEST_CASE(Span, ConstructorWithArray) {
  Array<int, 4> a1{11, 22, 33, 44};

  Span<int> s1(a1);
  CHECK_SPAN(s1, 11, 22, 33, 44);
  CHECK_EQ(s1.Data(), a1.Data());

  Span<int, 4> s2(a1);
  CHECK_SPAN(s2, 11, 22, 33, 44);
  CHECK_EQ(s1.Data(), a1.Data());
  s2[0] = 55;
  CHECK_SPAN(s2, 55, 22, 33, 44);
  CHECK_EQ(a1[0], 55);

  Array<int, 1> a2{111};
  Span<int> s3(a2);
  CHECK_SPAN(s3, 111);
}

TEST_CASE(Span, ConstructorWithConstArray) {
  const Array<int, 3> ca{4, 5, 6};
  Span<const int> s1(ca);
  CHECK_SPAN(s1, 4, 5, 6);
  CHECK_EQ(s1.Data(), ca.Data());

  Span<const int, 3> s2(ca);
  CHECK_SPAN(s2, 4, 5, 6);
  CHECK_EQ(s2.Data(), ca.Data());
}

using axio::Vector;

TEST_CASE(Span, ConstructorContainer) {
  Vector<int> v1{1, 2, 3, 4, 5};

  Span<int> s1(v1);
  CHECK_SPAN(s1, 1, 2, 3, 4, 5);
  CHECK_EQ(s1.Data(), v1.Data());
  s1[0] = 0;
  CHECK_SPAN(s1, 0, 2, 3, 4, 5);
  CHECK_EQ(v1[0], 0);

  std::vector<int> v2{1, 2, 3, 4};
  Span<int> s2(v2);
  CHECK_SPAN(s2, 1, 2, 3, 4);
  CHECK_EQ(s2.Data(), v2.data());
  s2[0] = 4;
  CHECK_SPAN(s2, 4, 2, 3, 4);
  CHECK_EQ(v2[0], 4);

  Vector<int> empty;
  Span<int> empty_span(empty);
  CHECK_TRUE(empty_span.IsEmpty());
}

TEST_CASE(Span, ConstructorConverting) {
  int arr[] = {1, 2, 3, 4};
  Span<int, 4> s1(arr);
  Span<int> s2(s1);
  CHECK_SPAN(s2, 1, 2, 3, 4);
  CHECK_EQ(s2.Data(), arr);

  Span<int> s3(arr, 3);
  Span<const int> s4 = s3;
  CHECK_SPAN(s4, 1, 2, 3);

  Span<int, 4> s5(arr);
  Span<const int, 4> s6 = s5;
  CHECK_SPAN(s6, 1, 2, 3, 4);

  Span<int> a(arr, 3);
  Span<int> b(a);
  CHECK_EQ(b.Data(), a.Data());
  CHECK_EQ(b.Size(), a.Size());

  Span<int> c;
  c = a;
  CHECK_EQ(c.Data(), a.Data());
  CHECK_EQ(c.Size(), a.Size());
}

TEST_CASE(Span, Observers) {
  int arr[] = {1, 2, 3, 4};

  Span<int> s(arr, AXIO_ARRAY_SIZE(arr));
  CHECK_FALSE(s.IsEmpty());
  CHECK_EQ(s.Data(), arr);
  CHECK_EQ(s.Size(), 4);
  CHECK_EQ(s.SizeBytes(), 4 * sizeof(int));

  Span<int> empty;
  CHECK_TRUE(empty.IsEmpty());
  CHECK_EQ(empty.Data(), nullptr);
  CHECK_EQ(empty.Size(), 0);
  CHECK_EQ(empty.SizeBytes(), 0);

  Span<int, 4> s1(arr);
  CHECK_EQ(s1.Size(), 4);
  CHECK_EQ(s1.SizeBytes(), 4 * sizeof(int));
  CHECK_EQ(s1.Data(), arr);
}

TEST_CASE(Span, ElementAccess) {
  int arr[] = {11, 22, 33, 44, 55};

  Span<int> s(arr);

  for (SizeT i = 0; i < s.Size(); ++i) {
    CHECK_EQ(s[i], (i + 1) * 11);
    CHECK_EQ(s.At(i), s[i]);
  }

  s[2] = 99;
  CHECK_SPAN(s, 11, 22, 99, 44, 55);
  arr[2] = 33;
  CHECK_SPAN(s, 11, 22, 33, 44, 55);

  CHECK_EQ(s.Front(), 11);
  CHECK_EQ(s.Back(), 55);

  s.Front() = 22;
  s.Back() = 44;
  CHECK_SPAN(s, 22, 22, 33, 44, 44);

  s.At(0) = 11;
  s.At(s.Size() - 1) = 55;
  CHECK_SPAN(s, 11, 22, 33, 44, 55);

  int one[] = {1};
  Span<int> s1(one);
  CHECK_EQ(s1.Front(), s1.Back());
  CHECK_EQ(&s1.Front(), &s1.Back());

  const int const_ints[] = {7, 8, 9};
  Span<const int> cs(const_ints, AXIO_ARRAY_SIZE(const_ints));
  CHECK_EQ(cs[0], 7);
  CHECK_EQ(cs.Front(), 7);
  CHECK_EQ(cs.Back(), 9);
  CHECK_EQ(cs.At(1), 8);
}

TEST_CASE(Span, Iterator) {
  int arr[] = {1, 2, 3, 4, 5, 6};
  Span<int> s(arr, 6);

  {
    int total = 0;
    for (auto x : s) {
      total += x;
    }
    CHECK_EQ(total, 21);
  }
  {
    int expected = 1;
    for (auto it = s.begin(); it != s.end(); ++it) {
      CHECK_EQ(*it, expected++);
    }
  }
  {
    int expected = 1;
    for (auto it = s.cbegin(); it != s.cend(); ++it) {
      CHECK_EQ(*it, expected++);
    }
  }
  {
    int expected = 6;
    for (auto it = s.rbegin(); it != s.rend(); ++it) {
      CHECK_EQ(*it, expected--);
    }
  }
  {
    int expected = 6;
    for (auto it = s.crbegin(); it != s.crend(); ++it) {
      CHECK_EQ(*it, expected--);
    }
  }
  {
    auto it = s.begin();
    CHECK_EQ(*(it + 0), 1);
    CHECK_EQ(*(it + 5), 6);
    it += 2;
    CHECK_EQ(*it, 3);
    it -= 1;
    CHECK_EQ(*it, 2);
    CHECK_EQ(s.end() - s.begin(), s.Size());
    CHECK_EQ(*(s.end() - 1), 6);
  }
  {
    *s.begin() = 9;
    CHECK_SPAN(s, 9, 2, 3, 4, 5, 6);
    arr[0] = 1;
    CHECK_SPAN(s, 1, 2, 3, 4, 5, 6);
  }
  {
    Span<int> empty;
    CHECK_EQ(empty.begin(), empty.end());
    CHECK_EQ(empty.cbegin(), empty.cend());
    CHECK_EQ(empty.rbegin(), empty.rend());
    CHECK_EQ(empty.crbegin(), empty.crend());
  }
  {
    const int ci[] = {5, 6, 7, 8, 9};
    Span<const int> cs(ci);
    int sum = 0;
    for (auto x : cs) {
      sum += x;
    }
    CHECK_EQ(sum, 35);
    static_assert(IsSame_V<decltype(*cs.begin()), const int&>);
  }
}

TEST_CASE(Span, First) {
  int arr[] = {1, 2, 3, 4, 5};
  Span<int, 5> s(arr);

  auto s1 = s.First<0>();
  static_assert(decltype(s1)::kExtent == 0);
  CHECK_EQ(s1.Size(), 0);
  CHECK_EQ(s1.Data(), arr);

  auto s2 = s.First<3>();
  static_assert(decltype(s2)::kExtent == 3);
  CHECK_SPAN(s2, 1, 2, 3);
  CHECK_EQ(s2.Data(), arr);

  auto s3 = s.First(0);
  static_assert(decltype(s3)::kExtent == axio::kDynamicExtent);
  CHECK_EQ(s3.Size(), 0);
  CHECK_EQ(s3.Data(), arr);

  auto s4 = s.First(4);
  static_assert(decltype(s4)::kExtent == axio::kDynamicExtent);
  CHECK_SPAN(s4, 1, 2, 3, 4);
  CHECK_EQ(s4.Data(), arr);
}

TEST_CASE(Span, Last) {
  int arr[] = {1, 2, 3, 4, 5, 6, 7, 8};
  Span<int, 8> s(arr);

  auto s1 = s.Last<0>();
  CHECK_EQ(s1.Size(), 0);
  CHECK_EQ(s1.Data(), arr + AXIO_ARRAY_SIZE(arr));

  auto s2 = s.Last<3>();
  CHECK_SPAN(s2, 6, 7, 8);

  auto s3 = s.Last(0);
  CHECK_EQ(s3.Size(), 0);

  auto s4 = s.Last(AXIO_ARRAY_SIZE(arr));
  CHECK_SPAN(s4, 1, 2, 3, 4, 5, 6, 7, 8);
}

TEST_CASE(Span_Subspan, Static) {
  int arr[] = {1, 2, 3, 4, 5, 6, 7};
  Span<int> s(arr);

  auto s1 = s.Subspan<0, 0>();
  static_assert(decltype(s1)::kExtent == 0);
  CHECK_EQ(s1.Size(), 0);
  CHECK_EQ(s1.Data(), arr);

  auto s2 = s.Subspan<0, 7>();
  static_assert(decltype(s2)::kExtent == 7);
  CHECK_SPAN(s2, 1, 2, 3, 4, 5, 6, 7);
  CHECK_EQ(s2.Data(), arr);

  auto s3 = s.Subspan<2, 3>();
  static_assert(decltype(s3)::kExtent == 3);
  CHECK_SPAN(s3, 3, 4, 5);
  CHECK_EQ(s3.Data(), arr + 2);

  auto s4 = s.Subspan<5, 0>();
  static_assert(decltype(s4)::kExtent == 0);
  CHECK_EQ(s4.Size(), 0);

  auto s5 = s.Subspan<0>();
  auto s6 = s.Subspan<2>();
  auto s7 = s.Subspan<7>();
  CHECK_SPAN(s5, 1, 2, 3, 4, 5, 6, 7);
  CHECK_SPAN(s6, 3, 4, 5, 6, 7);
  CHECK_EQ(s5.Data(), arr);
  CHECK_EQ(s6.Data(), arr + 2);
  CHECK_EQ(s7.Size(), 0);
}

TEST_CASE(Span_Subspan, Dynamic) {
  int arr[] = {1, 2, 3, 4, 5, 6, 7};
  Span<int> s(arr);

  auto s1 = s.Subspan(0, 0);
  CHECK_EQ(s1.Size(), 0);
  CHECK_EQ(s1.Data(), arr);

  auto s2 = s.Subspan(0, 7);
  CHECK_SPAN(s2, 1, 2, 3, 4, 5, 6, 7);
  CHECK_EQ(s2.Data(), arr);

  auto s3 = s.Subspan(2, 3);
  CHECK_SPAN(s3, 3, 4, 5);
  CHECK_EQ(s3.Data(), arr + 2);

  auto s4 = s.Subspan(5, 0);
  CHECK_EQ(s4.Size(), 0);

  auto s5 = s.Subspan(0);
  auto s6 = s.Subspan(2);
  auto s7 = s.Subspan(7);
  CHECK_SPAN(s5, 1, 2, 3, 4, 5, 6, 7);
  CHECK_SPAN(s6, 3, 4, 5, 6, 7);
  CHECK_EQ(s5.Data(), arr);
  CHECK_EQ(s6.Data(), arr + 2);
  CHECK_EQ(s7.Size(), 0);
}

TEST_CASE(Span, AsBytes) {
  {
    int arr[] = {1, 2, 3};
    Span<int> s(arr);
    auto bytes = s.AsBytes();
    static_assert(IsSame_V<decltype(bytes)::ElementType, const axio::Byte>);
    static_assert(decltype(bytes)::kExtent == axio::kDynamicExtent);
    CHECK_EQ(bytes.Size(), 3 * sizeof(int));
  }
  {
    int arr[3] = {1, 2, 3};
    Span<int, 3> s(arr);
    auto bytes = s.AsBytes();
    static_assert(decltype(bytes)::kExtent == 3 * sizeof(int));
    CHECK_EQ(bytes.Size(), 3 * sizeof(int));
  }
  {
    axio::UInt32 val = 0xDEADBEEF;
    Span<axio::UInt32> s(&val, 1);
    auto bytes = s.AsBytes();
    axio::UInt32 reconstructed = 0;
    std::memcpy(&reconstructed, bytes.Data(), sizeof(axio::UInt32));
    CHECK_EQ(reconstructed, val);
  }
}

TEST_CASE(Span, AsWritableBytes) {
  {
    uint8_t buf[] = {0x01, 0x02, 0x03};
    Span<uint8_t> s(buf, 3);
    auto wb = s.AsWritableBytes();
    static_assert(IsSame_V<decltype(wb)::ElementType, std::byte>);
    static_assert(decltype(wb)::kExtent == axio::kDynamicExtent);
    CHECK_EQ(wb.Size(), 3);
    wb[0] = std::byte{0xFF};
    CHECK_EQ(buf[0], 0xFF);
  }
  {
    int x = 0;
    Span<int> s(&x, 1);
    auto wb = s.AsWritableBytes();
    std::memset(wb.Data(), 0, wb.Size());
    CHECK_EQ(x, 0);
  }
}

TEST_CASE(Span, DeductionGuides) {
  {
    int arr[] = {1, 2, 3};
    Span s(arr);
    static_assert(IsSame_V<decltype(s), Span<int, 3>>);
  }

  {
    const int arr[] = {1, 2, 3};
    Span s(arr);
    static_assert(IsSame_V<decltype(s), Span<const int, 3>>);
  }

  {
    std::array<int, 4> a{1, 2, 3, 4};
    Span s(a);
    static_assert(IsSame_V<decltype(s), Span<int, 4>>);
  }

  {
    Array<int, 4> a{1, 2, 3, 4};
    Span s(a);
    static_assert(IsSame_V<decltype(s), Span<int, 4>>);
  }

  {
    const std::array<int, 4> a{1, 2, 3, 4};
    Span s(a);
    static_assert(IsSame_V<decltype(s), Span<const int, 4>>);
  }

  {
    const Array<int, 4> a{1, 2, 3, 4};
    Span s(a);
    static_assert(IsSame_V<decltype(s), Span<const int, 4>>);
  }

  {
    Vector<int> v{1, 2, 3};
    Span s(v);
    static_assert(IsSame_V<decltype(s), Span<int>>);
  }
}

TEST_CASE(Span, Split) {
  int arr[] = {1, 2, 3, 4, 5, 6, 7};
  Span s(arr);

  {
    auto [head, tail] = s.Split<0>();
    static_assert(decltype(head)::kExtent == 0);
    static_assert(decltype(tail)::kExtent == 7);
    CHECK_TRUE(head.IsEmpty());
    CHECK_SPAN(tail, 1, 2, 3, 4, 5, 6, 7);
  }

  {
    auto [head, tail] = s.Split<3>();
    static_assert(decltype(head)::kExtent == 3);
    static_assert(decltype(tail)::kExtent == 4);
    CHECK_SPAN(head, 1, 2, 3);
    CHECK_SPAN(tail, 4, 5, 6, 7);
  }

  {
    auto [head, tail] = s.Split<7>();
    static_assert(decltype(head)::kExtent == 7);
    static_assert(decltype(tail)::kExtent == 0);
    CHECK_SPAN(head, 1, 2, 3, 4, 5, 6, 7);
    CHECK_TRUE(tail.IsEmpty());
  }

  {
    auto [head, tail] = s.Split(0);
    static_assert(decltype(head)::kExtent == axio::kDynamicExtent);
    static_assert(decltype(tail)::kExtent == axio::kDynamicExtent);
    CHECK_TRUE(head.IsEmpty());
    CHECK_SPAN(tail, 1, 2, 3, 4, 5, 6, 7);
  }

  {
    auto [head, tail] = s.Split(3);
    static_assert(decltype(head)::kExtent == axio::kDynamicExtent);
    static_assert(decltype(tail)::kExtent == axio::kDynamicExtent);
    CHECK_SPAN(head, 1, 2, 3);
    CHECK_SPAN(tail, 4, 5, 6, 7);
  }

  {
    auto [head, tail] = s.Split(7);
    static_assert(decltype(head)::kExtent == axio::kDynamicExtent);
    static_assert(decltype(tail)::kExtent == axio::kDynamicExtent);
    CHECK_SPAN(head, 1, 2, 3, 4, 5, 6, 7);
    CHECK_TRUE(tail.IsEmpty());
  }
}

TEST_CASE(Span, As) {
  using axio::Byte;

  {
    int arr[] = {1, 2, 3};
    Span<int> s1(arr);
    Span<const Byte> bytes = s1.AsBytes();
    Span<const int> s2 = bytes.As<int>();

    CHECK_SPAN(s1, 1, 2, 3);
    CHECK_SPAN(s2, 1, 2, 3);
  }

  {
    struct Point {
      int x, y;

      bool operator==(const Point& other) const noexcept {
        return x == other.x && y == other.y;
      }
    };

    Point arr[] = {{1, 2}, {2, 3}, {3, 4}, {4, 5}};
    Span<Point> s1(arr);
    Span<const Byte> bytes = s1.AsBytes();
    Span<const Point> s2 = bytes.As<Point>();

    CHECK_SPAN(s1, Point{1, 2}, Point{2, 3}, Point{3, 4}, Point{4, 5});
    CHECK_SPAN(s2, Point{1, 2}, Point{2, 3}, Point{3, 4}, Point{4, 5});
  }
}

TEST_CASE(Span, AsWritable) {
  using axio::Byte;

  {
    int arr[] = {1, 2, 3};
    Span<int> s1(arr);
    Span<Byte> bytes = s1.AsWritableBytes();
    Span<int> s2 = bytes.AsWritable<int>();

    s2[0] = 3;
    s1[2] = 1;

    CHECK_SPAN(s1, 3, 2, 1);
    CHECK_SPAN(s2, 3, 2, 1);
  }

  {
    struct Point {
      int x, y;

      bool operator==(const Point& other) const noexcept {
        return x == other.x && y == other.y;
      }
    };

    Point arr[] = {{1, 2}, {2, 3}, {3, 4}, {4, 5}};
    Span<Point> s1(arr);
    Span<Byte> bytes = s1.AsWritableBytes();
    Span<Point> s2 = bytes.AsWritable<Point>();

    CHECK_SPAN(s1, Point{1, 2}, Point{2, 3}, Point{3, 4}, Point{4, 5});
    CHECK_SPAN(s2, Point{1, 2}, Point{2, 3}, Point{3, 4}, Point{4, 5});
  }
}