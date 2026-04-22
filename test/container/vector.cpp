#include <simpletest/simpletest.hpp>

#include <axio/container/vector.hpp>

#include <list>
#include <sstream>
#include <vector>

using axio::Vector;

using IntVector = Vector<int>;

namespace {
template <typename T, typename A, typename U>
axio::Bool Expect(const Vector<T, A>& v, std::initializer_list<U> l) {
  if (v.Size() != l.size()) {
    return false;
  }
  auto vb = v.begin();
  auto ve = v.end();
  auto lb = l.begin();
  for (; vb != ve; ++vb, ++lb) {
    if (!(*vb == *lb)) {
      return false;
    }
  }
  return true;
}
}  // namespace

#define CHECK_VECTOR(v, ...) CHECK_TRUE(Expect(v, __VA_ARGS__));

#define CHECK_IS_FULLY_EMPTY(v) \
  CHECK_TRUE(v.IsEmpty());      \
  CHECK_EQ(v.Data(), nullptr);  \
  CHECK_EQ(v.Size(), 0);        \
  CHECK_EQ(v.Capacity(), 0);

TEST_CASE(IntVector, DefaultConstructor) {
  IntVector v;
  CHECK_IS_FULLY_EMPTY(v);
}

TEST_CASE(IntVector, AllocatorConstructor) {
  axio::Allocator<int> allocator;
  IntVector v(allocator);
  CHECK_IS_FULLY_EMPTY(v);
}

TEST_CASE(IntVector, InputItConstructor) {
  std::stringstream ss("10 20 30 40");
  std::istream_iterator<int> first(ss);
  std::istream_iterator<int> last;
  IntVector v(first, last);
  CHECK_VECTOR(v, {10, 20, 30, 40});
}

TEST_CASE(IntVector, ForwardItConstructor) {
  {
    IntVector v1{5, 1, 4, 9, 8};
    IntVector v2(v1.begin(), v1.end());
    CHECK_VECTOR(v2, {5, 1, 4, 9, 8});
  }
  {
    std::vector<int> v1{1, 2, 3, 4, 5};
    IntVector v2(v1.begin(), v1.end());
    CHECK_VECTOR(v2, {1, 2, 3, 4, 5});
  }
  {
    std::list<int> l{4, 1, 5, 6, 3};
    IntVector v(l.begin(), l.end());
    CHECK_VECTOR(v, {4, 1, 5, 6, 3});
  }
}

TEST_CASE(IntVector, CountConstructor) {
  IntVector v(5);
  CHECK_FALSE(v.IsEmpty());
  CHECK_EQ(v.Size(), 5);
  CHECK_EQ(v.Capacity(), 5);
  CHECK_VECTOR(v, {int(), int(), int(), int(), int()});

  IntVector v1(0);
  CHECK_IS_FULLY_EMPTY(v1);

  axio::Allocator<int> a;
  IntVector v2(3, a);
  CHECK_VECTOR(v2, {int(), int(), int()});

  // IntVector v3(-1); // should crash!
}

TEST_CASE(IntVector, CountConstructWithValue) {
  IntVector v(5, 3);
  CHECK_VECTOR(v, {3, 3, 3, 3, 3});

  IntVector v1(0, 3);
  CHECK_IS_FULLY_EMPTY(v1);

  axio::Allocator<int> a;
  IntVector v2(5, 4, a);
  CHECK_VECTOR(v2, {4, 4, 4, 4, 4});

  // IntVector v3(-1, 5);  // should crash!
}

TEST_CASE(IntVector, InitListConstructor) {
  IntVector v{1, 2, 3, 4, 5};
  CHECK_EQ(v.Size(), 5);
  CHECK_EQ(v.Capacity(), 5);
  CHECK_VECTOR(v, {1, 2, 3, 4, 5});

  IntVector v1({});
  CHECK_IS_FULLY_EMPTY(v1);

  axio::Allocator<int> a;
  IntVector v2({1, 2, 3}, a);
  CHECK_VECTOR(v2, {1, 2, 3});
}

TEST_CASE(IntVector, CopyConstructor) {
  {
    IntVector original{4, 2, 5, 1};

    IntVector copy(original);
    CHECK_VECTOR(copy, {4, 2, 5, 1});
    copy[2] = 8;
    CHECK_VECTOR(copy, {4, 2, 8, 1});
    CHECK_VECTOR(original, {4, 2, 5, 1});

    axio::Allocator<int> a;
    IntVector copy1(original, a);
    CHECK_VECTOR(copy1, {4, 2, 5, 1});
  }
  {
    IntVector empty;

    IntVector copy(empty);
    CHECK_IS_FULLY_EMPTY(copy);

    axio::Allocator<int> a;
    IntVector copy1(empty, a);
    CHECK_IS_FULLY_EMPTY(copy1);
  }
}

TEST_CASE(IntVector, MoveConstructor) {
  {
    IntVector original{1, 2, 3, 4};
    IntVector moved(axio::Move(original));
    CHECK_EQ(moved.Capacity(), 4);
    CHECK_VECTOR(moved, {1, 2, 3, 4});
    CHECK_IS_FULLY_EMPTY(original);
  }
  {
    axio::Allocator<int> a;
    IntVector original{1, 2, 3, 4};
    IntVector moved(axio::Move(original), a);
    CHECK_EQ(moved.Capacity(), 4);
    CHECK_VECTOR(moved, {1, 2, 3, 4});
    CHECK_IS_FULLY_EMPTY(original);
  }
}

TEST_CASE(IntVector, CopyAssign) {
  IntVector v;

  {
    IntVector vt{1, 2, 3, 4, 5, 6, 7, 8};
    v = vt;
    CHECK_VECTOR(v, {1, 2, 3, 4, 5, 6, 7, 8});
  }
  {
    IntVector vt;
    v = vt;
    CHECK_TRUE(v.IsEmpty());
  }
  {
    IntVector vt{5, 1, 4, 7, 1, 4, 3};
    v = vt;
    CHECK_VECTOR(v, {5, 1, 4, 7, 1, 4, 3});
  }
  {
    v = v;
    CHECK_VECTOR(v, {5, 1, 4, 7, 1, 4, 3});
  }
}

TEST_CASE(IntVector, MoveAssign) {
  IntVector v;

  {
    IntVector vt{1, 2, 3, 4, 5};
    v = axio::Move(vt);
    CHECK_VECTOR(v, {1, 2, 3, 4, 5});
    CHECK_IS_FULLY_EMPTY(vt);
  }
  {
    IntVector vt;
    v = axio::Move(vt);
    CHECK_IS_FULLY_EMPTY(v);
    CHECK_IS_FULLY_EMPTY(vt);
  }
  {
    IntVector vt{1, 2, 3, 4, 5};
    v = axio::Move(vt);
    CHECK_VECTOR(v, {1, 2, 3, 4, 5});
    CHECK_IS_FULLY_EMPTY(vt);
  }
  {
    v = axio::Move(v);
    CHECK_VECTOR(v, {1, 2, 3, 4, 5});
  }
}

TEST_CASE(IntVector, AssignInitList) {
  {
    IntVector v;
    v = {4, 5, 1, 4};
    CHECK_VECTOR(v, {4, 5, 1, 4});
    v = {6, 1, 7, 8, 1, 4, 6, 9, 1, 0, 4, 6, 4, 3};
    CHECK_VECTOR(v, {6, 1, 7, 8, 1, 4, 6, 9, 1, 0, 4, 6, 4, 3});
    v = {3, 5, 2, 7, 5};
    CHECK_VECTOR(v, {3, 5, 2, 7, 5});
    v = {5, 4, 6, 7, 1, 5, 7, 1, 6, 1};
    CHECK_VECTOR(v, {5, 4, 6, 7, 1, 5, 7, 1, 6, 1});
  }
  {
    IntVector v;
    v.Assign({2, 4, 5, 1});
    CHECK_VECTOR(v, {2, 4, 5, 1});
  }
}

TEST_CASE(IntVector, AssignWithInputIt) {
  IntVector v;
  {
    std::stringstream ss("10 20 30 40");
    std::istream_iterator<int> first(ss);
    std::istream_iterator<int> last;
    v.Assign(first, last);
    CHECK_VECTOR(v, {10, 20, 30, 40});
  }
  {
    std::stringstream ss("5 6 6 8 9 1 2 5 6 7 1");
    std::istream_iterator<int> first(ss);
    std::istream_iterator<int> last;
    v.Assign(first, last);
    CHECK_VECTOR(v, {5, 6, 6, 8, 9, 1, 2, 5, 6, 7, 1});
  }
  {
    std::stringstream ss("3 8 9 1 6 7 1");
    std::istream_iterator<int> first(ss);
    std::istream_iterator<int> last;
    v.Assign(first, last);
    CHECK_VECTOR(v, {3, 8, 9, 1, 6, 7, 1});
  }
}

TEST_CASE(IntVector, AssignWithForwardIt) {
  IntVector v;

  {
    std::list<int> l{1, 5, 6, 1, 4};
    v.Assign(l.begin(), l.end());
    CHECK_VECTOR(v, {1, 5, 6, 1, 4});
  }
  {
    std::vector<int> vt{6, 1, 4, 2, 5, 7, 1, 4, 4};
    v.Assign(vt.begin(), vt.end());
    CHECK_VECTOR(v, {6, 1, 4, 2, 5, 7, 1, 4, 4});
  }
  {
    IntVector vt{1, 2, 3, 4, 5};
    v.Assign(vt.begin(), vt.end());
    CHECK_VECTOR(v, {1, 2, 3, 4, 5});

    v.Assign(v.begin(), v.end());
    CHECK_VECTOR(v, {1, 2, 3, 4, 5});

    v.Assign(v.begin() + 2, v.end());
    CHECK_VECTOR(v, {3, 4, 5});
  }
}

TEST_CASE(IntVector, CountAssign) {
  IntVector v;
  v.Assign(3, 5);
  CHECK_VECTOR(v, {5, 5, 5});
  v.Assign(10, 4);
  CHECK_VECTOR(v, {4, 4, 4, 4, 4, 4, 4, 4, 4, 4});
  v.Assign(5, 8);
  CHECK_VECTOR(v, {8, 8, 8, 8, 8});
  v.Assign(8, 0);
  CHECK_VECTOR(v, {0, 0, 0, 0, 0, 0, 0, 0});
}

TEST_CASE(IntVector, ElementAccess) {
  {
    IntVector v{5, 1, 4, 2};
    CHECK_EQ(v[0], 5);
    CHECK_EQ(v[1], 1);
    CHECK_EQ(v[2], 4);
    CHECK_EQ(v[3], 2);
    // CHECK_EQ(v[4], 2); // should crash

    CHECK_FALSE(v.IsEmpty());
    CHECK_EQ(v.Size(), 4);
    CHECK_EQ(v.Capacity(), 4);

    CHECK_EQ(*v.Data(), 5);
    CHECK_EQ(*(v.Data() + 2), 4);

    CHECK_EQ(v.Front(), 5);
    CHECK_EQ(v.Back(), 2);

    v.At(3) = 1;
    CHECK_VECTOR(v, {5, 1, 4, 1});
  }
  {
    const IntVector v{5, 1, 4, 2};
    CHECK_EQ(v[0], 5);
    CHECK_EQ(v[1], 1);
    CHECK_EQ(v[2], 4);
    CHECK_EQ(v[3], 2);
    // CHECK_EQ(v[4], 2); // should crash

    CHECK_FALSE(v.IsEmpty());
    CHECK_EQ(v.Size(), 4);
    CHECK_EQ(v.Capacity(), 4);

    CHECK_EQ(*v.Data(), 5);
    CHECK_EQ(*(v.Data() + 2), 4);

    CHECK_EQ(v.Front(), 5);
    CHECK_EQ(v.Back(), 2);
  }
}

TEST_CASE(IntVector, Iterator) {
  {
    IntVector v(4, 2);
    for (int x : v) {
      CHECK_EQ(x, 2);
    }
  }

  {
    const IntVector v(4, 2);
    for (const int x : v) {
      CHECK_EQ(x, 2);
    }
  }

  {
    IntVector v(4, 2);
    for (auto b = v.cbegin(), e = v.cend(); b != e; ++b) {
      CHECK_EQ(*b, 2);
    }
  }
}

TEST_CASE(IntVector, ReverseIterator) {
  {
    IntVector v{1, 2, 3, 4};
    int i = 4;
    for (auto b = v.rbegin(), e = v.rend(); b != e; ++b) {
      CHECK_EQ(*b, i--);
    }
  }
  {
    const IntVector v{1, 2, 3, 4};
    int i = 4;
    for (auto b = v.rbegin(), e = v.rend(); b != e; ++b) {
      CHECK_EQ(*b, i--);
    }
  }
  {
    IntVector v{1, 2, 3, 4};
    int i = 4;
    for (auto b = v.crbegin(), e = v.crend(); b != e; ++b) {
      CHECK_EQ(*b, i--);
    }
  }
}

TEST_CASE(IntVector, Resize) {
  IntVector v;
  v.Resize(4);
  CHECK_VECTOR(v, {0, 0, 0, 0});

  v.Resize(9);
  CHECK_VECTOR(v, {0, 0, 0, 0, 0, 0, 0, 0, 0});

  v.Resize(5);
  CHECK_VECTOR(v, {0, 0, 0, 0, 0});

  v.Resize(12, 3);
  CHECK_VECTOR(v, {0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, 3});

  v.Resize(3, 4);
  CHECK_VECTOR(v, {0, 0, 0});
}

TEST_CASE(IntVector, Clear) {
  IntVector v;
  v.Clear();
  CHECK_IS_FULLY_EMPTY(v);

  v = {1, 2, 3, 4, 5};
  v.Clear();
  CHECK_EQ(v.Size(), 0);
}

TEST_CASE(IntVector, Reserve_Shrink) {
  IntVector v{1, 2, 3, 4, 5, 6, 7, 8};
  v.Reserve(3);
  CHECK_EQ(v.Size(), 8);
  CHECK_EQ(v.Capacity(), 8);
  CHECK_VECTOR(v, {1, 2, 3, 4, 5, 6, 7, 8});

  v.Reserve(13);
  CHECK_EQ(v.Size(), 8);
  CHECK_EQ(v.Capacity(), 13);
  CHECK_VECTOR(v, {1, 2, 3, 4, 5, 6, 7, 8});

  v.Shrink();
  CHECK_EQ(v.Size(), 8);
  CHECK_EQ(v.Capacity(), 8);
  CHECK_VECTOR(v, {1, 2, 3, 4, 5, 6, 7, 8});
}

TEST_CASE(IntVector, Push) {
  IntVector v;
  v.Push(1);
  v.Push(2);
  v.Push(3);
  CHECK_VECTOR(v, {1, 2, 3});

  v.Push(5);
  v.Push(4);
  v.Push(1);
  v.Push(7);
  v.Push(9);
  CHECK_VECTOR(v, {1, 2, 3, 5, 4, 1, 7, 9});

  v.Push(1) = 9;
  CHECK_VECTOR(v, {1, 2, 3, 5, 4, 1, 7, 9, 9});
}

TEST_CASE(IntVector, AppendCount) {
  IntVector v;
  v.Append(4, 1);
  CHECK_VECTOR(v, {1, 1, 1, 1});
  v.Append(8, 3);
  CHECK_VECTOR(v, {1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3});
  v.Append(3, 2);
  CHECK_VECTOR(v, {1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2});
  v.Append(4, 8);
  CHECK_VECTOR(v, {1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 8, 8, 8, 8});
}

TEST_CASE(IntVector, AppendInitList) {
  IntVector v;
  v.Append({1, 2, 3, 4});
  CHECK_VECTOR(v, {1, 2, 3, 4});
  v.Append({4, 5, 1, 4, 2, 3, 4, 5});
  CHECK_VECTOR(v, {1, 2, 3, 4, 4, 5, 1, 4, 2, 3, 4, 5});
  v.Append({3, 1, 5});
  CHECK_VECTOR(v, {1, 2, 3, 4, 4, 5, 1, 4, 2, 3, 4, 5, 3, 1, 5});
}

TEST_CASE(IntVector, AppendInputIterator) {
  IntVector v;
  {
    std::stringstream ss("10 20 30 40");
    std::istream_iterator<int> first(ss);
    std::istream_iterator<int> last;
    v.Append(first, last);
    CHECK_VECTOR(v, {10, 20, 30, 40});
  }
  {
    std::stringstream ss("1 4 5 8 1 3 5 1 9");
    std::istream_iterator<int> first(ss);
    std::istream_iterator<int> last;
    v.Append(first, last);
    CHECK_VECTOR(v, {10, 20, 30, 40, 1, 4, 5, 8, 1, 3, 5, 1, 9});
  }
}

TEST_CASE(IntVector, AppendForwardIterator) {
  {
    std::list<int> l{1, 2, 3, 4};

    IntVector v;
    v.Append(l.begin(), l.end());
    CHECK_VECTOR(v, {1, 2, 3, 4});

    l = {3, 4, 1, 5, 1, 9, 8, 6};
    v.Append(l.begin(), l.end());
    CHECK_VECTOR(v, {1, 2, 3, 4, 3, 4, 1, 5, 1, 9, 8, 6});
  }
  {
    IntVector vt{1, 5, 2, 4};
    IntVector v;
    v.Append(vt.begin(), vt.end());
    CHECK_VECTOR(v, {1, 5, 2, 4});

    vt = {5, 1, 4, 6, 1, 4, 8, 4};
    v.Append(vt.begin(), vt.end());
    CHECK_VECTOR(v, {1, 5, 2, 4, 5, 1, 4, 6, 1, 4, 8, 4});
  }
  {
    IntVector v{1, 5, 2, 4};
    v.Append(v.begin(), v.end());
    CHECK_VECTOR(v, {1, 5, 2, 4, 1, 5, 2, 4});

    v.Append(v.begin() + 2, v.end() - 2);
    CHECK_VECTOR(v, {1, 5, 2, 4, 1, 5, 2, 4, 2, 4, 1, 5})
  }
}

TEST_CASE(IntVector, RemoveOneElement) {
  IntVector v{4, 1, 5, 6, 3, 2};
  {
    auto it = v.Remove(v.begin());
    CHECK_EQ(*it, 1);
    CHECK_VECTOR(v, {1, 5, 6, 3, 2});
  }
  {
    auto it = v.Remove(v.end() - 1);
    CHECK_EQ(it, v.end());
    CHECK_VECTOR(v, {1, 5, 6, 3});
  }
  {
    auto it = v.Remove(v.begin() + 2);
    CHECK_EQ(*it, 3);
    CHECK_VECTOR(v, {1, 5, 3});
  }
  {
    v = {6, 1, 4, 5, 8, 9};
    auto it = v.Remove(v.begin() + 4);
    CHECK_EQ(*it, 9);
    CHECK_VECTOR(v, {6, 1, 4, 5, 9});

    it = v.Remove(v.begin() + 2);
    CHECK_EQ(*it, 5);
    CHECK_VECTOR(v, {6, 1, 5, 9});
  }
}

TEST_CASE(IntVector, RemoveRange) {
  IntVector v{6, 1, 5, 1, 3, 4, 6, 1, 8, 9, 6, 4, 5};

  auto it = v.Remove(v.begin(), v.begin() + 4);
  CHECK_EQ(*it, 3);
  CHECK_VECTOR(v, {3, 4, 6, 1, 8, 9, 6, 4, 5});

  it = v.Remove(v.begin() + 6, v.end());
  CHECK_EQ(it, v.end());
  CHECK_VECTOR(v, {3, 4, 6, 1, 8, 9});

  v = {3, 4, 5, 1, 5, 6, 2, 3, 7, 8};
  it = v.Remove(v.begin() + 3, v.begin() + 8);
  CHECK_EQ(*it, 7);
  CHECK_VECTOR(v, {3, 4, 5, 7, 8});
}

TEST_CASE(IntVector, Pop) {
  IntVector v{10, 20, 30, 40};
  v.Pop();
  CHECK_VECTOR(v, {10, 20, 30});
}

TEST_CASE(IntVector, Emplace) {
  IntVector v;

  auto it = v.Emplace(v.begin(), 3);
  CHECK_VECTOR(v, {3});
  CHECK_EQ(*it, 3);

  for (int i = 0; i < 20; ++i) {
    it = v.Emplace(v.begin() + i / 2, i);
    CHECK_EQ(it, v.begin() + i / 2);
    CHECK_EQ(*it, i);
  }
  CHECK_VECTOR(v, {1,  3,  5,  7,  9, 11, 13, 15, 17, 19, 18,
                   16, 14, 12, 10, 8, 6,  4,  2,  0,  3});

  v = {1, 3, 4, 1, 5};
  v.Emplace(v.begin(), v[4]);
  CHECK_VECTOR(v, {5, 1, 3, 4, 1, 5});
}

TEST_CASE(IntVector, InsertInputIterator) {
  IntVector v;

  {
    std::stringstream ss("10 20 30 40");
    std::istream_iterator<int> first(ss);
    std::istream_iterator<int> last;
    auto it = v.Insert(v.begin(), first, last);
    CHECK_EQ(it, v.begin());
    CHECK_EQ(*it, 10);
    CHECK_VECTOR(v, {10, 20, 30, 40});
  }
  {
    std::stringstream ss("91 8 4 6 5 1 3 2 14 2 34");
    std::istream_iterator<int> first(ss);
    std::istream_iterator<int> last;
    auto it = v.Insert(v.begin() + 2, first, last);
    CHECK_EQ(it, v.begin() + 2);
    CHECK_EQ(*it, 91);
    CHECK_VECTOR(v, {10, 20, 91, 8, 4, 6, 5, 1, 3, 2, 14, 2, 34, 30, 40});
  }
}

TEST_CASE(IntVector, InsertCount) {
  IntVector v;

  {
    auto it = v.Insert(v.begin(), 4, 3);
    CHECK_EQ(it, v.begin());
    CHECK_EQ(*it, 3);
    CHECK_VECTOR(v, {3, 3, 3, 3});
  }
  {
    auto it = v.Insert(v.begin() + 2, 5, 9);
    CHECK_EQ(it, v.begin() + 2);
    CHECK_EQ(*it, 9);
    CHECK_VECTOR(v, {3, 3, 9, 9, 9, 9, 9, 3, 3});
  }
  {
    auto it = v.Insert(v.end(), 3, 2);
    CHECK_EQ(it, v.end() - 3);
    CHECK_EQ(*it, 2);
    CHECK_VECTOR(v, {3, 3, 9, 9, 9, 9, 9, 3, 3, 2, 2, 2});
  }
  {
    auto it = v.Insert(v.end(), 0, 2);
    CHECK_EQ(it, v.end());
    CHECK_VECTOR(v, {3, 3, 9, 9, 9, 9, 9, 3, 3, 2, 2, 2});
  }

  v.Reserve(24);
  v = {1, 2, 3, 4, 5, 6, 7};
  {
    auto it = v.Insert(v.begin() + 3, 7, 3);
    CHECK_EQ(it, v.begin() + 3);
    CHECK_EQ(*it, 3);
    CHECK_VECTOR(v, {1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 5, 6, 7});
  }
  {
    auto it = v.Insert(v.begin() + 3, 4, 9);
    CHECK_EQ(it, v.begin() + 3);
    CHECK_EQ(*it, 9);
    CHECK_VECTOR(v, {1, 2, 3, 9, 9, 9, 9, 3, 3, 3, 3, 3, 3, 3, 4, 5, 6, 7});
  }

  v = {1, 3, 4, 1, 5};

  v.Insert(v.begin(), 3, v[4]);
  CHECK_VECTOR(v, {5, 5, 5, 1, 3, 4, 1, 5});
}

TEST_CASE(IntVector, InsertForwardIterator) {
  IntVector v;

  {
    std::list<int> l{5, 1, 4, 7, 8, 1};
    auto it = v.Insert(v.begin(), l.begin(), l.end());
    CHECK_EQ(it, v.begin());
    CHECK_EQ(*it, 5);
    CHECK_VECTOR(v, {5, 1, 4, 7, 8, 1});
  }
  {
    std::vector<int> vt{1, 5, 5, 1, 4, 2, 3};
    auto it = v.Insert(v.begin() + 2, vt.begin(), vt.end());
    CHECK_EQ(it, v.begin() + 2);
    CHECK_EQ(*it, 1);
    CHECK_VECTOR(v, {5, 1, 1, 5, 5, 1, 4, 2, 3, 4, 7, 8, 1});
  }
  {
    IntVector vt{8, 1, 9, 0, 2};
    auto it = v.Insert(v.begin() + 7, vt.begin(), vt.end());
    CHECK_EQ(it, v.begin() + 7);
    CHECK_EQ(*it, 8);
    CHECK_VECTOR(v, {5, 1, 1, 5, 5, 1, 4, 8, 1, 9, 0, 2, 2, 3, 4, 7, 8, 1});
  }
  {
    IntVector vt{8, 1, 9, 0, 2};
    auto it = v.Insert(v.end(), vt.begin(), vt.end());
    CHECK_EQ(it, v.end() - vt.Size());
    CHECK_EQ(*it, 8);
    CHECK_VECTOR(v, {5, 1, 1, 5, 5, 1, 4, 8, 1, 9, 0, 2,
                     2, 3, 4, 7, 8, 1, 8, 1, 9, 0, 2});
  }

  v = {1, 2, 3, 4, 5};

  {
    IntVector vt{8, 1, 9, 0};
    auto it = v.Insert(v.begin(), vt.begin(), vt.end());
    CHECK_EQ(it, v.begin());
    CHECK_EQ(*it, 8);
    CHECK_VECTOR(v, {8, 1, 9, 0, 1, 2, 3, 4, 5});
  }
  {
    IntVector vt{8, 1, 9, 0};
    auto it = v.Insert(v.begin(), vt.begin(), vt.end());
    CHECK_EQ(it, v.begin());
    CHECK_EQ(*it, 8);
    CHECK_VECTOR(v, {8, 1, 9, 0, 8, 1, 9, 0, 1, 2, 3, 4, 5});
  }
}

TEST_CASE(IntVector, Comparision) {
  {
    IntVector v1{11, 22, 33, 44};
    IntVector v2{11, 22, 33, 44};
    CHECK_EQ(v1, v2);
  }
  {
    IntVector v1{11, 22, 33, 44, 55};
    IntVector v2{11, 22, 33, 44, 22};
    CHECK_NE(v1, v2);

    v1 = {11, 22, 33};
    v2 = {11, 22};
    CHECK_NE(v1, v2);
  }
  {
    IntVector v1{11, 22, 33};
    IntVector v2{11, 22};
    CHECK_TRUE(v1 > v2);
    CHECK_TRUE(v1 >= v2);

    CHECK_TRUE(v2 < v1);
    CHECK_TRUE(v2 <= v1);

    v1 = {11, 22, 33};
    v2 = {11, 22, 33};
    CHECK_TRUE(v1 >= v2);
    CHECK_TRUE(v2 <= v1);
  }
}

////////////////////////////////////////////////////////////////////

class NonTrivial {
 public:
  static int alive_count;

  char* data;
  size_t size;

  NonTrivial() : size(0) {
    data = new char[1]{'\0'};
    alive_count++;
  }

  NonTrivial(const char* s) {
    size = s ? std::strlen(s) : 0;
    data = new char[size + 1];
    if (s)
      std::memcpy(data, s, size + 1);
    else
      data[0] = '\0';
    alive_count++;
  }

  NonTrivial(const NonTrivial& other) : size(other.size) {
    data = new char[size + 1];
    std::memcpy(data, other.data, size + 1);
    alive_count++;
  }

  NonTrivial(NonTrivial&& other) noexcept : data(other.data), size(other.size) {
    other.data = nullptr;
    other.size = 0;
    alive_count++;
  }

  NonTrivial& operator=(const char* s) {
    delete[] data;

    if (s) {
      size = std::strlen(s);
      data = new char[size + 1];
      std::memcpy(data, s, size + 1);
    } else {
      size = 0;
      data = new char[1]{'\0'};
    }
    return *this;
  }

  NonTrivial& operator=(const NonTrivial& other) {
    if (this != &other) {
      char* new_data = new char[other.size + 1];
      std::memcpy(new_data, other.data, other.size + 1);
      delete[] data;
      data = new_data;
      size = other.size;
    }
    return *this;
  }

  NonTrivial& operator=(NonTrivial&& other) noexcept {
    if (this != &other) {
      delete[] data;
      data = other.data;
      size = other.size;
      other.data = nullptr;
      other.size = 0;
    }
    return *this;
  }

  ~NonTrivial() {
    if (data) {
      delete[] data;
      data = nullptr;
      size = 0;
    }
    alive_count--;
  }

  bool operator==(const NonTrivial& other) const {
    if (size != other.size)
      return false;
    if (data == other.data)
      return true;
    return std::memcmp(data, other.data, size) == 0;
  }

  bool operator!=(const NonTrivial& other) const { return !(*this == other); }

  bool operator==(const char* s) const {
    if (!s)
      return data == nullptr;
    if (!data)
      return false;
    size_t s_len = std::strlen(s);
    return (size == s_len) && (std::memcmp(data, s, size) == 0);
  }

  bool operator<(const NonTrivial& other) const {
    if (!data)
      return other.data != nullptr;
    if (!other.data)
      return false;
    return std::strcmp(data, other.data) < 0;
  }

  bool operator<=(const NonTrivial& other) const { return !(other < *this); }

  bool operator>(const NonTrivial& other) const { return other < *this; }

  bool operator>=(const NonTrivial& other) const { return !(*this < other); }

  bool operator!=(const char* s) const { return !(*this == s); }

  friend std::istream& operator>>(std::istream& is, NonTrivial& obj) {
    std::string temp;
    if (is >> temp) {
      obj = NonTrivial(temp.c_str());
    }
    return is;
  }
};

int NonTrivial::alive_count = 0;

static_assert(!axio::IsTriviallyCopyable<NonTrivial>::value,
              "NonTrivial must not be trivially copyable!");

static_assert(!axio::IsTriviallyDestructible<NonTrivial>::value,
              "NonTrivial must not be trivially destructible!");

#define CHECK_ALIVE_COUNT(val) CHECK_EQ(NonTrivial::alive_count, val)

using NT = NonTrivial;
using NTVector = Vector<NT>;

TEST_CASE(NTVector, DefaultConstructor) {
  NTVector v;
  CHECK_IS_FULLY_EMPTY(v);
}

TEST_CASE(NTVector, AllocatorConstructor) {
  axio::Allocator<NT> allocator;
  NTVector v(allocator);
  CHECK_IS_FULLY_EMPTY(v);
}

TEST_CASE(NTVector, InputItConstructor) {
  std::stringstream ss("hello world foo bar");  // 4 NTs
  std::istream_iterator<NT> first(ss);          // 1 NT
  std::istream_iterator<NT> last;               // 1 NT
  NTVector v(first, last);
  CHECK_VECTOR(v, {"hello", "world", "foo", "bar"});
  CHECK_ALIVE_COUNT(6);
}

TEST_CASE(NTVector, ForwardItConstructor) {
  {
    NTVector v1{"foo", "foo", "bar", "baz", "abc"};
    CHECK_ALIVE_COUNT(5);
    NTVector v2(v1.begin(), v1.end());
    CHECK_ALIVE_COUNT(10);
    CHECK_VECTOR(v2, {"foo", "foo", "bar", "baz", "abc"});
  }
  CHECK_ALIVE_COUNT(0);
  {
    std::list<NT> l{"foo", "foo", "bar", "baz", "abc"};
    CHECK_ALIVE_COUNT(5);
    NTVector v(l.begin(), l.end());
    CHECK_ALIVE_COUNT(10);
    CHECK_VECTOR(v, {"foo", "foo", "bar", "baz", "abc"});
  }
  CHECK_ALIVE_COUNT(0);
  {
    Vector<const char*> v1{"hello", "world", "c++"};
    CHECK_ALIVE_COUNT(0);
    NTVector v(v1.begin(), v1.end());
    CHECK_ALIVE_COUNT(3);
    CHECK_VECTOR(v, {"hello", "world", "c++"});
  }
  CHECK_ALIVE_COUNT(0);
  {
    std::vector<NT> v1{"hello", "world", "c++"};
    CHECK_ALIVE_COUNT(3);
    NTVector v(v1.begin(), v1.end());
    CHECK_ALIVE_COUNT(6);
    CHECK_VECTOR(v, {"hello", "world", "c++"});
  }
  CHECK_ALIVE_COUNT(0);
}

TEST_CASE(NTVector, CountConstructor) {
  {
    NTVector v(5);
    CHECK_FALSE(v.IsEmpty());
    CHECK_EQ(v.Size(), 5);
    CHECK_EQ(v.Capacity(), 5);
    CHECK_VECTOR(v, {"", "", "", "", ""});
    CHECK_ALIVE_COUNT(5);
  }
  CHECK_ALIVE_COUNT(0);
  {
    NTVector v(0);
    CHECK_IS_FULLY_EMPTY(v);
    CHECK_ALIVE_COUNT(0);
  }
  {
    axio::Allocator<NT> a;
    NTVector v(3, a);
    CHECK_VECTOR(v, {"", "", ""});
    CHECK_ALIVE_COUNT(3);
  }
  CHECK_ALIVE_COUNT(0);

  {
    // NTVector v(-1); // should crash!
  }
}

TEST_CASE(NTVector, CountConstructorWithValue) {
  CHECK_ALIVE_COUNT(0);
  {
    NTVector v(5, "hello");
    CHECK_VECTOR(v, {"hello", "hello", "hello", "hello", "hello"});
    CHECK_ALIVE_COUNT(5);
  }
  CHECK_ALIVE_COUNT(0);
  {
    NTVector v(0, "aaa");
    CHECK_IS_FULLY_EMPTY(v);
    CHECK_ALIVE_COUNT(0);
  }
  {
    axio::Allocator<NT> a;
    NTVector v(5, "abc", a);
    CHECK_VECTOR(v, {"abc", "abc", "abc", "abc", "abc"});
    CHECK_ALIVE_COUNT(5);
  }
  CHECK_ALIVE_COUNT(0);
  {
    // NTVector v(-1, "...");  // should crash!
  }
}

TEST_CASE(NTVector, InitListConstructor) {
  {
    NTVector v({});
    CHECK_IS_FULLY_EMPTY(v);
    CHECK_ALIVE_COUNT(0);
  }
  {
    NTVector v{"abc", "lol", "qwerty", "foo"};
    CHECK_EQ(v.Size(), 4);
    CHECK_EQ(v.Capacity(), 4);
    CHECK_VECTOR(v, {"abc", "lol", "qwerty", "foo"});
    CHECK_ALIVE_COUNT(4);
  }
  CHECK_ALIVE_COUNT(0);
  {
    axio::Allocator<NT> a;
    NTVector v({"abc", "abc", "abc"}, a);
    CHECK_VECTOR(v, {"abc", "abc", "abc"});
    CHECK_ALIVE_COUNT(3);
  }
  CHECK_ALIVE_COUNT(0);
}

TEST_CASE(NTVector, CopyConstructor) {
  {
    NTVector original{"on", "off", "foo", "bar"};
    NTVector copy(original);
    CHECK_ALIVE_COUNT(8);

    CHECK_VECTOR(copy, {"on", "off", "foo", "bar"});
    copy[2] = "baz";
    CHECK_VECTOR(copy, {"on", "off", "baz", "bar"});
    CHECK_VECTOR(original, {"on", "off", "foo", "bar"});
  }
  CHECK_ALIVE_COUNT(0);
  {
    NTVector original{"on", "off", "foo", "bar"};
    axio::Allocator<NT> a;
    NTVector copy(original, a);
    CHECK_VECTOR(copy, {"on", "off", "foo", "bar"});
    CHECK_ALIVE_COUNT(8);
  }
  CHECK_ALIVE_COUNT(0);
  {
    NTVector empty;

    NTVector copy(empty);
    CHECK_IS_FULLY_EMPTY(copy);

    axio::Allocator<NT> a;
    NTVector copy1(empty, a);
    CHECK_IS_FULLY_EMPTY(copy1);
    CHECK_ALIVE_COUNT(0);
  }
}

TEST_CASE(NTVector, MoveConstructor) {
  {
    NTVector original{"foo", "bar", "baz"};
    CHECK_ALIVE_COUNT(3);
    NTVector moved(axio::Move(original));
    CHECK_ALIVE_COUNT(3);
    CHECK_EQ(moved.Capacity(), 3);
    CHECK_VECTOR(moved, {"foo", "bar", "baz"});
    CHECK_IS_FULLY_EMPTY(original);
  }
  CHECK_ALIVE_COUNT(0);
  {
    axio::Allocator<int> a;
    NTVector original{"foo", "bar", "baz"};
    CHECK_ALIVE_COUNT(3);
    NTVector moved(axio::Move(original), a);
    CHECK_ALIVE_COUNT(3);
    CHECK_EQ(moved.Capacity(), 3);
    CHECK_VECTOR(moved, {"foo", "bar", "baz"});
    CHECK_IS_FULLY_EMPTY(original);
  }
  CHECK_ALIVE_COUNT(0);
}

TEST_CASE(NTVector, CopyAssign) {
  NTVector v;
  CHECK_ALIVE_COUNT(0);
  {
    NTVector vt{"aa", "bb", "cc", "dd", "ee", "ff", "gg", "hh"};
    v = vt;
    CHECK_VECTOR(v, {"aa", "bb", "cc", "dd", "ee", "ff", "gg", "hh"});
    CHECK_ALIVE_COUNT(16);
  }
  CHECK_ALIVE_COUNT(8);
  {
    NTVector empty;
    v = empty;
    CHECK_TRUE(v.IsEmpty());
    CHECK_ALIVE_COUNT(0);
  }
  CHECK_ALIVE_COUNT(0);
  {
    NTVector vt{"11", "22", "33", "44", "55", "66", "77"};
    v = vt;
    CHECK_VECTOR(v, {"11", "22", "33", "44", "55", "66", "77"});
    CHECK_ALIVE_COUNT(14);
  }
  CHECK_ALIVE_COUNT(7);
  {
    v = v;
    CHECK_VECTOR(v, {"11", "22", "33", "44", "55", "66", "77"});
  }
  CHECK_ALIVE_COUNT(7);
}

TEST_CASE(NTVector, MoveAssign) {
  NTVector v;
  CHECK_ALIVE_COUNT(0);
  {
    NTVector vt{"11", "22", "33", "44", "55"};
    v = axio::Move(vt);
    CHECK_VECTOR(v, {"11", "22", "33", "44", "55"});
    CHECK_IS_FULLY_EMPTY(vt);
    CHECK_ALIVE_COUNT(5);
  }
  CHECK_ALIVE_COUNT(5);
  {
    NTVector empty;
    v = axio::Move(empty);
    CHECK_IS_FULLY_EMPTY(v);
    CHECK_IS_FULLY_EMPTY(empty);
    CHECK_ALIVE_COUNT(0);
  }
  {
    NTVector vt{"foo", "bar", "baz", "abc", "xyz"};
    v = axio::Move(vt);
    CHECK_VECTOR(v, {"foo", "bar", "baz", "abc", "xyz"});
    CHECK_IS_FULLY_EMPTY(vt);
    CHECK_ALIVE_COUNT(5);
  }
  CHECK_ALIVE_COUNT(5);
  {
    v = axio::Move(v);
    CHECK_VECTOR(v, {"foo", "bar", "baz", "abc", "xyz"});
    CHECK_ALIVE_COUNT(5);
  }
  CHECK_ALIVE_COUNT(5);
}

TEST_CASE(NTVector, AssignInitList) {
  {
    NTVector v;
    v = {"4", "5", "1", "4"};
    CHECK_ALIVE_COUNT(4);
    CHECK_VECTOR(v, {"4", "5", "1", "4"});
    v = {"6", "1", "7", "8", "1", "4", "6", "9", "1", "0", "4", "6", "4", "3"};
    CHECK_ALIVE_COUNT(14);
    CHECK_VECTOR(v, {"6", "1", "7", "8", "1", "4", "6", "9", "1", "0", "4", "6",
                     "4", "3"});
    v = {"3", "5", "2", "7", "5"};
    CHECK_ALIVE_COUNT(5);
    CHECK_VECTOR(v, {"3", "5", "2", "7", "5"});
    v = {"5", "4", "6", "7", "1", "5", "7", "1", "6", "1"};
    CHECK_ALIVE_COUNT(10);
    CHECK_VECTOR(v, {"5", "4", "6", "7", "1", "5", "7", "1", "6", "1"});
  }
  CHECK_ALIVE_COUNT(0);
  {
    NTVector v;
    v.Assign({"2", "4", "5", "1"});
    CHECK_VECTOR(v, {"2", "4", "5", "1"});
  }
  CHECK_ALIVE_COUNT(0);
}

TEST_CASE(NTVector, AssignWithInputIt) {
  NTVector v;
  CHECK_ALIVE_COUNT(0);
  {
    std::stringstream ss("10 20 30 40");
    std::istream_iterator<NT> first(ss);  // +1
    std::istream_iterator<NT> last;       // +1
    v.Assign(first, last);
    CHECK_VECTOR(v, {"10", "20", "30", "40"});
    CHECK_ALIVE_COUNT(4 + 1 + 1);
  }
  CHECK_ALIVE_COUNT(4);
  {
    std::stringstream ss("5 6 6 8 9 1 2 5 6 7 1");
    std::istream_iterator<NT> first(ss);
    std::istream_iterator<NT> last;
    v.Assign(first, last);
    CHECK_VECTOR(v, {"5", "6", "6", "8", "9", "1", "2", "5", "6", "7", "1"});
    CHECK_ALIVE_COUNT(11 + 1 + 1);
  }
  CHECK_ALIVE_COUNT(11);
  {
    std::stringstream ss("3 8 9 1 6 7 1");
    std::istream_iterator<NT> first(ss);
    std::istream_iterator<NT> last;
    v.Assign(first, last);
    CHECK_VECTOR(v, {"3", "8", "9", "1", "6", "7", "1"});
    CHECK_ALIVE_COUNT(7 + 1 + 1);
  }
  CHECK_ALIVE_COUNT(7);
}

TEST_CASE(NTVector, AssignWithForwardIt) {
  NTVector v;
  CHECK_ALIVE_COUNT(0);

  {
    std::list<NT> l{"1", "5", "6", "1", "4"};
    v.Assign(l.begin(), l.end());
    CHECK_VECTOR(v, {"1", "5", "6", "1", "4"});
    CHECK_ALIVE_COUNT(10);
  }
  CHECK_ALIVE_COUNT(5);
  {
    std::vector<NT> vt{"6", "1", "4", "2", "5", "7", "1", "4", "4"};
    v.Assign(vt.begin(), vt.end());
    CHECK_VECTOR(v, {"6", "1", "4", "2", "5", "7", "1", "4", "4"});
    CHECK_ALIVE_COUNT(18);
  }
  CHECK_ALIVE_COUNT(9);
  {
    NTVector vt{"1", "2", "3", "4", "5"};
    v.Assign(vt.begin(), vt.end());
    CHECK_VECTOR(v, {"1", "2", "3", "4", "5"});
    CHECK_ALIVE_COUNT(10);
  }
  CHECK_ALIVE_COUNT(5);
  {
    v.Assign(v.begin(), v.end());
    CHECK_VECTOR(v, {"1", "2", "3", "4", "5"});
    CHECK_ALIVE_COUNT(5);

    v.Assign(v.begin() + 2, v.end());
    CHECK_VECTOR(v, {"3", "4", "5"});
    CHECK_ALIVE_COUNT(3);
  }
  CHECK_ALIVE_COUNT(3);
  {
    std::list<const char*> l{"apple", "mango", "tomato", "watermelon"};
    v.Assign(l.begin(), l.end());
    CHECK_VECTOR(v, {"apple", "mango", "tomato", "watermelon"});
    CHECK_ALIVE_COUNT(4);
  }
  CHECK_ALIVE_COUNT(4);
}

TEST_CASE(NTVector, CountAssign) {
  NTVector v;
  v.Assign(3, NT("foo"));
  CHECK_VECTOR(v, {"foo", "foo", "foo"});
  CHECK_ALIVE_COUNT(3);

  v.Assign(10, NT("bar"));
  CHECK_VECTOR(v, {"bar", "bar", "bar", "bar", "bar", "bar", "bar", "bar",
                   "bar", "bar"});
  CHECK_ALIVE_COUNT(10);

  v.Assign(5, NT("..."));
  CHECK_VECTOR(v, {"...", "...", "...", "...", "..."});
  CHECK_ALIVE_COUNT(5);

  v.Assign(8, "baz");
  CHECK_VECTOR(v, {"baz", "baz", "baz", "baz", "baz", "baz", "baz", "baz"});
  CHECK_ALIVE_COUNT(8);
}

TEST_CASE(NTVector, ElementAccess) {
  {
    NTVector v{"55", "11", "44", "22"};
    CHECK_EQ(v[0], "55");
    CHECK_EQ(v[1], "11");
    CHECK_EQ(v[2], "44");
    CHECK_EQ(v[3], "22");
    CHECK_ALIVE_COUNT(4);
    // CHECK_EQ(v[4], "22"); // should crash

    CHECK_FALSE(v.IsEmpty());
    CHECK_EQ(v.Size(), 4);
    CHECK_EQ(v.Capacity(), 4);

    CHECK_EQ(*v.Data(), "55");
    CHECK_EQ(*(v.Data() + 2), "44");

    CHECK_EQ(v.Front(), "55");
    CHECK_EQ(v.Back(), "22");

    v.At(3) = "11";
    CHECK_VECTOR(v, {"55", "11", "44", "11"});
  }
  CHECK_ALIVE_COUNT(0);
  {
    const NTVector v{"55", "11", "44", "22"};
    CHECK_EQ(v[0], "55");
    CHECK_EQ(v[1], "11");
    CHECK_EQ(v[2], "44");
    CHECK_EQ(v[3], "22");
    CHECK_ALIVE_COUNT(4);
    // CHECK_EQ(v[4], "22"); // should crash

    CHECK_FALSE(v.IsEmpty());
    CHECK_EQ(v.Size(), 4);
    CHECK_EQ(v.Capacity(), 4);

    CHECK_EQ(*v.Data(), "55");
    CHECK_EQ(*(v.Data() + 2), "44");

    CHECK_EQ(v.Front(), "55");
    CHECK_EQ(v.Back(), "22");
  }
  CHECK_ALIVE_COUNT(0);
}

TEST_CASE(NTVector, Iterator) {
  {
    NTVector v(4, "aa");
    CHECK_ALIVE_COUNT(4);
    for (NT& nt : v) {
      CHECK_EQ(nt, "aa");
      nt = "bb";
    }
    CHECK_VECTOR(v, {"bb", "bb", "bb", "bb"});
  }
  CHECK_ALIVE_COUNT(0);
  {
    NTVector v(4, "aa");
    CHECK_ALIVE_COUNT(4);
    for (const NT& nt : v) {
      CHECK_EQ(nt, "aa");
    }
  }
  CHECK_ALIVE_COUNT(0);
  {
    NTVector v(4, "abc");
    CHECK_ALIVE_COUNT(4);
    for (auto b = v.cbegin(), e = v.cend(); b != e; ++b) {
      CHECK_EQ(*b, "abc");
    }
  }
  CHECK_ALIVE_COUNT(0);
}

TEST_CASE(NTVector, ReverseIterator) {
  {
    NTVector v{"1", "2", "3", "4"};
    CHECK_ALIVE_COUNT(4);
    char c = '4';
    for (auto b = v.rbegin(), e = v.rend(); b != e; ++b) {
      CHECK_EQ((*b).data[0], c--);
    }
  }
  CHECK_ALIVE_COUNT(0);
  {
    const NTVector v{"1", "2", "3", "4"};
    CHECK_ALIVE_COUNT(4);
    char c = '4';
    for (auto b = v.rbegin(), e = v.rend(); b != e; ++b) {
      CHECK_EQ((*b).data[0], c--);
    }
  }
  CHECK_ALIVE_COUNT(0);
  {
    NTVector v{"1", "2", "3", "4"};
    CHECK_ALIVE_COUNT(4);
    char c = '4';
    for (auto b = v.crbegin(), e = v.crend(); b != e; ++b) {
      CHECK_EQ((*b).data[0], c--);
    }
  }
  CHECK_ALIVE_COUNT(0);
}

TEST_CASE(NTVector, Resize) {
  {
    NTVector v;
    CHECK_ALIVE_COUNT(0);
    v.Resize(4);
    CHECK_VECTOR(v, {"", "", "", ""});
    CHECK_ALIVE_COUNT(4);

    v.Resize(9);
    CHECK_VECTOR(v, {"", "", "", "", "", "", "", "", ""});
    CHECK_ALIVE_COUNT(9);

    v.Resize(5);
    CHECK_VECTOR(v, {"", "", "", "", ""});
    CHECK_ALIVE_COUNT(5);

    v.Resize(12, "3");
    CHECK_VECTOR(v, {"", "", "", "", "", "3", "3", "3", "3", "3", "3", "3"});
    CHECK_ALIVE_COUNT(12);

    v.Resize(3, "4");
    CHECK_VECTOR(v, {"", "", ""});
    CHECK_ALIVE_COUNT(3);
  }
  CHECK_ALIVE_COUNT(0);
}

TEST_CASE(NTVector, Clear) {
  NTVector v;
  v.Clear();
  CHECK_IS_FULLY_EMPTY(v);
  CHECK_ALIVE_COUNT(0);

  v = {"111", "222", "333", "444", "555"};
  CHECK_ALIVE_COUNT(5);
  v.Clear();
  CHECK_ALIVE_COUNT(0);
  CHECK_EQ(v.Size(), 0);
}

TEST_CASE(NTVector, Reserve_Shrink) {
  NTVector v{"1", "2", "3", "4", "5", "6", "7", "8"};
  CHECK_ALIVE_COUNT(8);
  v.Reserve(3);
  CHECK_ALIVE_COUNT(8);
  CHECK_EQ(v.Size(), 8);
  CHECK_EQ(v.Capacity(), 8);
  CHECK_VECTOR(v, {"1", "2", "3", "4", "5", "6", "7", "8"});

  v.Reserve(13);
  CHECK_ALIVE_COUNT(8);
  CHECK_EQ(v.Size(), 8);
  CHECK_EQ(v.Capacity(), 13);
  CHECK_VECTOR(v, {"1", "2", "3", "4", "5", "6", "7", "8"});

  v.Shrink();
  CHECK_ALIVE_COUNT(8);
  CHECK_EQ(v.Size(), 8);
  CHECK_EQ(v.Capacity(), 8);
  CHECK_VECTOR(v, {"1", "2", "3", "4", "5", "6", "7", "8"});
}

TEST_CASE(NTVector, Push) {
  {
    NTVector v;
    CHECK_ALIVE_COUNT(0);
    v.Push("hello");
    CHECK_ALIVE_COUNT(1);
    v.Push("world");
    CHECK_ALIVE_COUNT(2);
    v.Push(NT("c++"));
    CHECK_ALIVE_COUNT(3);
    CHECK_VECTOR(v, {"hello", "world", "c++"});

    v.Push("11");
    CHECK_ALIVE_COUNT(4);
    v.Push("22");
    CHECK_ALIVE_COUNT(5);
    v.Push("33");
    CHECK_ALIVE_COUNT(6);
    v.Push("44");
    CHECK_ALIVE_COUNT(7);
    v.Push("55");
    CHECK_ALIVE_COUNT(8);
    v.Push("66");
    CHECK_ALIVE_COUNT(9);
    CHECK_VECTOR(v,
                 {"hello", "world", "c++", "11", "22", "33", "44", "55", "66"});

    v.Push("...") = "abc";
    CHECK_ALIVE_COUNT(10);
    CHECK_VECTOR(v, {"hello", "world", "c++", "11", "22", "33", "44", "55",
                     "66", "abc"});
  }
  CHECK_ALIVE_COUNT(0);
}

TEST_CASE(NTVector, AppendCount) {
  {
    NTVector v;
    CHECK_ALIVE_COUNT(0);

    v.Append(4, "a");
    CHECK_VECTOR(v, {"a", "a", "a", "a"});
    CHECK_ALIVE_COUNT(4);

    v.Append(8, "b");
    CHECK_VECTOR(v,
                 {"a", "a", "a", "a", "b", "b", "b", "b", "b", "b", "b", "b"});
    CHECK_ALIVE_COUNT(12);

    v.Append(3, "c");
    CHECK_VECTOR(v, {"a", "a", "a", "a", "b", "b", "b", "b", "b", "b", "b", "b",
                     "c", "c", "c"});
    CHECK_ALIVE_COUNT(15);

    v.Append(4, "d");
    CHECK_VECTOR(v, {"a", "a", "a", "a", "b", "b", "b", "b", "b", "b", "b", "b",
                     "c", "c", "c", "d", "d", "d", "d"});
    CHECK_ALIVE_COUNT(19);

    {
      const NT value("ee");
      v.Append(3, value);
      CHECK_VECTOR(v,
                   {"a", "a", "a", "a", "b", "b", "b", "b", "b",  "b",  "b",
                    "b", "c", "c", "c", "d", "d", "d", "d", "ee", "ee", "ee"});
      CHECK_ALIVE_COUNT(23);
    }
    CHECK_ALIVE_COUNT(22);
  }
  CHECK_ALIVE_COUNT(0);
}

TEST_CASE(NTVector, AppendInitList) {
  {
    NTVector v;
    CHECK_ALIVE_COUNT(0);
    v.Append({"1", "2", "3", "4"});
    CHECK_ALIVE_COUNT(4);
    CHECK_VECTOR(v, {"1", "2", "3", "4"});
    v.Append({"4", "5", "1", "4", "2", "3", "4", "5"});
    CHECK_ALIVE_COUNT(12);
    CHECK_VECTOR(v,
                 {"1", "2", "3", "4", "4", "5", "1", "4", "2", "3", "4", "5"});
    v.Append({"3", "1", "5"});
    CHECK_ALIVE_COUNT(15);
    CHECK_VECTOR(v, {"1", "2", "3", "4", "4", "5", "1", "4", "2", "3", "4", "5",
                     "3", "1", "5"});
  }
  CHECK_ALIVE_COUNT(0);
}

TEST_CASE(NTVector, AppendInputIterator) {
  NTVector v;
  CHECK_ALIVE_COUNT(0);
  {
    std::stringstream ss("10 20 30 40");
    std::istream_iterator<NT> first(ss);
    std::istream_iterator<NT> last;
    v.Append(first, last);
    CHECK_VECTOR(v, {"10", "20", "30", "40"});
    CHECK_ALIVE_COUNT(4 + 1 + 1);
  }
  CHECK_ALIVE_COUNT(4);
  {
    std::stringstream ss("1 4 5 8 1 3 5 1 9");
    std::istream_iterator<NT> first(ss);
    std::istream_iterator<NT> last;
    v.Append(first, last);
    CHECK_VECTOR(v, {"10", "20", "30", "40", "1", "4", "5", "8", "1", "3", "5",
                     "1", "9"});
    CHECK_ALIVE_COUNT(13 + 1 + 1);
  }
  CHECK_ALIVE_COUNT(13);
}

TEST_CASE(NTVector, AppendForwardIterator) {
  {
    std::list<NT> l{"1", "2", "3", "4"};
    CHECK_ALIVE_COUNT(4);

    NTVector v;
    v.Append(l.begin(), l.end());
    CHECK_ALIVE_COUNT(8);
    CHECK_VECTOR(v, {"1", "2", "3", "4"});

    l = {"3", "4", "1", "5", "1", "9", "8", "6"};
    CHECK_ALIVE_COUNT(12);
    v.Append(l.begin(), l.end());
    CHECK_ALIVE_COUNT(20);
    CHECK_VECTOR(v,
                 {"1", "2", "3", "4", "3", "4", "1", "5", "1", "9", "8", "6"});
  }
  CHECK_ALIVE_COUNT(0);
  {
    NTVector vt{"1", "5", "2", "4"};
    CHECK_ALIVE_COUNT(4);
    NTVector v;
    v.Append(vt.begin(), vt.end());
    CHECK_VECTOR(v, {"1", "5", "2", "4"});
    CHECK_ALIVE_COUNT(8);

    vt = {"5", "1", "4", "6", "1", "4", "8", "4"};
    CHECK_ALIVE_COUNT(12);
    v.Append(vt.begin(), vt.end());
    CHECK_VECTOR(v,
                 {"1", "5", "2", "4", "5", "1", "4", "6", "1", "4", "8", "4"});
    CHECK_ALIVE_COUNT(20);
  }
  CHECK_ALIVE_COUNT(0);
  {
    NTVector v{"1", "5", "2", "4"};
    CHECK_ALIVE_COUNT(4);
    v.Append(v.begin(), v.end());
    CHECK_VECTOR(v, {"1", "5", "2", "4", "1", "5", "2", "4"});
    CHECK_ALIVE_COUNT(8);

    v.Append(v.begin() + 2, v.end() - 2);
    CHECK_VECTOR(v,
                 {"1", "5", "2", "4", "1", "5", "2", "4", "2", "4", "1", "5"})
    CHECK_ALIVE_COUNT(12);
  }
  CHECK_ALIVE_COUNT(0);
}

TEST_CASE(NTVector, RemoveOneElement) {
  NTVector v{"4", "1", "5", "6", "3", "2"};
  CHECK_ALIVE_COUNT(6);
  {
    auto it = v.Remove(v.begin());
    CHECK_EQ(*it, "1");
    CHECK_ALIVE_COUNT(5);
    CHECK_VECTOR(v, {"1", "5", "6", "3", "2"});
  }
  {
    auto it = v.Remove(v.end() - 1);
    CHECK_EQ(it, v.end());
    CHECK_ALIVE_COUNT(4);
    CHECK_VECTOR(v, {"1", "5", "6", "3"});
  }
  {
    auto it = v.Remove(v.begin() + 2);
    CHECK_EQ(*it, "3");
    CHECK_ALIVE_COUNT(3);
    CHECK_VECTOR(v, {"1", "5", "3"});
  }
  {
    v = {"6", "1", "4", "5", "8", "9"};
    CHECK_ALIVE_COUNT(6);
    auto it = v.Remove(v.begin() + 4);
    CHECK_EQ(*it, "9");
    CHECK_ALIVE_COUNT(5);
    CHECK_VECTOR(v, {"6", "1", "4", "5", "9"});

    it = v.Remove(v.begin() + 2);
    CHECK_EQ(*it, "5");
    CHECK_VECTOR(v, {"6", "1", "5", "9"});
  }
}

TEST_CASE(NTVector, RemoveRange) {
  NTVector v{"6", "1", "5", "1", "3", "4", "6", "1", "8", "9", "6", "4", "5"};
  CHECK_ALIVE_COUNT(13);

  auto it = v.Remove(v.begin(), v.begin() + 4);
  CHECK_EQ(*it, "3");
  CHECK_ALIVE_COUNT(9);
  CHECK_VECTOR(v, {"3", "4", "6", "1", "8", "9", "6", "4", "5"});

  it = v.Remove(v.begin() + 6, v.end());
  CHECK_EQ(it, v.end());
  CHECK_ALIVE_COUNT(6);
  CHECK_VECTOR(v, {"3", "4", "6", "1", "8", "9"});

  v = {"3", "4", "5", "1", "5", "6", "2", "3", "7", "8"};
  CHECK_ALIVE_COUNT(10);
  it = v.Remove(v.begin() + 3, v.begin() + 8);
  CHECK_EQ(*it, "7");
  CHECK_ALIVE_COUNT(5);
  CHECK_VECTOR(v, {"3", "4", "5", "7", "8"});
}

TEST_CASE(NTVector, Pop) {
  NTVector v{"10", "20", "30", "40"};
  CHECK_ALIVE_COUNT(4);
  v.Pop();
  CHECK_ALIVE_COUNT(3);
  CHECK_VECTOR(v, {"10", "20", "30"});
}

TEST_CASE(NTVector, Emplace) {
  NTVector v;

  auto it = v.Emplace(v.begin(), "foo");
  CHECK_EQ(*it, "foo");
  CHECK_VECTOR(v, {"foo"});
  CHECK_ALIVE_COUNT(1);

  static constexpr const char* texts[]{"foo", "bar", "baz", "abc"};

  for (int i = 0; i < 20; ++i) {
    const char* value = texts[static_cast<size_t>(i) % AXIO_ARRAY_SIZE(texts)];
    it = v.Emplace(v.begin() + i / 2, value);
    CHECK_EQ(it, v.begin() + i / 2);
    CHECK_EQ(*it, value);
  }

  CHECK_VECTOR(v, {"bar", "abc", "bar", "abc", "bar", "abc", "bar",
                   "abc", "bar", "abc", "baz", "foo", "baz", "foo",
                   "baz", "foo", "baz", "foo", "baz", "foo", "foo"});
  CHECK_ALIVE_COUNT(21);

  v = {"1", "2", "3", "4", "5"};
  CHECK_ALIVE_COUNT(5);
  v.Emplace(v.begin(), v[4]);
  CHECK_VECTOR(v, {"5", "1", "2", "3", "4", "5"});
  CHECK_ALIVE_COUNT(6);
}

TEST_CASE(NTVector, InsertInputIterator) {
  NTVector v;

  {
    std::stringstream ss("10 20 30 40");
    std::istream_iterator<NT> first(ss);
    std::istream_iterator<NT> last;
    auto it = v.Insert(v.begin(), first, last);
    CHECK_EQ(it, v.begin());
    CHECK_EQ(*it, "10");
    CHECK_VECTOR(v, {"10", "20", "30", "40"});
    CHECK_ALIVE_COUNT(4 + 1 + 1);
  }
  CHECK_ALIVE_COUNT(4);
  {
    std::stringstream ss("91 8 4 6 5 1 3 2 14 2 34");
    std::istream_iterator<NT> first(ss);
    std::istream_iterator<NT> last;
    auto it = v.Insert(v.begin() + 2, first, last);
    CHECK_EQ(it, v.begin() + 2);
    CHECK_EQ(*it, "91");
    CHECK_VECTOR(v, {"10", "20", "91", "8", "4", "6", "5", "1", "3", "2", "14",
                     "2", "34", "30", "40"});
    CHECK_ALIVE_COUNT(15 + 1 + 1);
  }
  CHECK_ALIVE_COUNT(15);
}

TEST_CASE(NTVector, InsertCount) {
  NTVector v;

  {
    auto it = v.Insert(v.begin(), 4, "foo");
    CHECK_EQ(it, v.begin());
    CHECK_EQ(*it, "foo");
    CHECK_VECTOR(v, {"foo", "foo", "foo", "foo"});
    CHECK_ALIVE_COUNT(4);
  }
  {
    auto it = v.Insert(v.begin() + 2, 5, "bar");
    CHECK_EQ(it, v.begin() + 2);
    CHECK_EQ(*it, "bar");
    CHECK_VECTOR(
        v, {"foo", "foo", "bar", "bar", "bar", "bar", "bar", "foo", "foo"});
    CHECK_ALIVE_COUNT(9);
  }
  {
    auto it = v.Insert(v.end(), 3, "baz");
    CHECK_EQ(it, v.end() - 3);
    CHECK_EQ(*it, "baz");
    CHECK_VECTOR(v, {"foo", "foo", "bar", "bar", "bar", "bar", "bar", "foo",
                     "foo", "baz", "baz", "baz"});
    CHECK_ALIVE_COUNT(12);
  }
  {
    auto it = v.Insert(v.end(), 0, "abc");
    CHECK_EQ(it, v.end());
    CHECK_VECTOR(v, {"foo", "foo", "bar", "bar", "bar", "bar", "bar", "foo",
                     "foo", "baz", "baz", "baz"});
    CHECK_ALIVE_COUNT(12);
  }

  v.Reserve(24);
  v = {"1", "2", "3", "4", "5", "6", "7"};
  CHECK_ALIVE_COUNT(7);
  {
    auto it = v.Insert(v.begin() + 3, 7, "3");
    CHECK_EQ(it, v.begin() + 3);
    CHECK_EQ(*it, "3");
    CHECK_VECTOR(v, {"1", "2", "3", "3", "3", "3", "3", "3", "3", "3", "4", "5",
                     "6", "7"});
    CHECK_ALIVE_COUNT(14);
  }
  {
    auto it = v.Insert(v.begin() + 3, 4, "9");
    CHECK_EQ(it, v.begin() + 3);
    CHECK_EQ(*it, "9");
    CHECK_VECTOR(v, {"1", "2", "3", "9", "9", "9", "9", "3", "3", "3", "3", "3",
                     "3", "3", "4", "5", "6", "7"});
    CHECK_ALIVE_COUNT(18);
  }

  v = {"1", "3", "4", "1", "5"};
  CHECK_ALIVE_COUNT(5);

  v.Insert(v.begin(), 3, v[4]);
  CHECK_VECTOR(v, {"5", "5", "5", "1", "3", "4", "1", "5"});
  CHECK_ALIVE_COUNT(8);
}

TEST_CASE(NTVector, InsertForwardIterator) {
  NTVector v;

  {
    std::list<NT> l{"5", "1", "4", "7", "8", "1"};
    auto it = v.Insert(v.begin(), l.begin(), l.end());
    CHECK_EQ(it, v.begin());
    CHECK_EQ(*it, "5");
    CHECK_VECTOR(v, {"5", "1", "4", "7", "8", "1"});
    CHECK_ALIVE_COUNT(12);
  }
  CHECK_ALIVE_COUNT(6);
  {
    std::vector<NT> vt{"1", "5", "5", "1", "4", "2", "3"};
    auto it = v.Insert(v.begin() + 2, vt.begin(), vt.end());
    CHECK_EQ(it, v.begin() + 2);
    CHECK_EQ(*it, "1");
    CHECK_VECTOR(
        v, {"5", "1", "1", "5", "5", "1", "4", "2", "3", "4", "7", "8", "1"});
    CHECK_ALIVE_COUNT(20);
  }
  CHECK_ALIVE_COUNT(13);
  {
    NTVector vt{"8", "1", "9", "0", "2"};
    auto it = v.Insert(v.begin() + 7, vt.begin(), vt.end());
    CHECK_EQ(it, v.begin() + 7);
    CHECK_EQ(*it, "8");
    CHECK_VECTOR(v, {"5", "1", "1", "5", "5", "1", "4", "8", "1", "9", "0", "2",
                     "2", "3", "4", "7", "8", "1"});
    CHECK_ALIVE_COUNT(23);
  }
  CHECK_ALIVE_COUNT(18);
  {
    Vector<const char*> vt{"8", "1", "9", "0", "2"};
    auto it = v.Insert(v.end(), vt.begin(), vt.end());
    CHECK_EQ(it, v.end() - vt.Size());
    CHECK_EQ(*it, "8");
    CHECK_VECTOR(v, {"5", "1", "1", "5", "5", "1", "4", "8", "1", "9", "0", "2",
                     "2", "3", "4", "7", "8", "1", "8", "1", "9", "0", "2"});
    CHECK_ALIVE_COUNT(23);
  }
  CHECK_ALIVE_COUNT(23);

  v = {"1", "2", "3", "4", "5"};
  CHECK_ALIVE_COUNT(5);

  {
    NTVector vt{"8", "1", "9", "0"};
    CHECK_ALIVE_COUNT(9);
    auto it = v.Insert(v.begin(), vt.begin(), vt.end());
    CHECK_EQ(it, v.begin());
    CHECK_EQ(*it, "8");
    CHECK_VECTOR(v, {"8", "1", "9", "0", "1", "2", "3", "4", "5"});
    CHECK_ALIVE_COUNT(13);
  }
  CHECK_ALIVE_COUNT(9);
  {
    NTVector vt{"8", "1", "9", "0"};
    CHECK_ALIVE_COUNT(13);
    auto it = v.Insert(v.begin(), vt.begin(), vt.end());
    CHECK_EQ(it, v.begin());
    CHECK_EQ(*it, "8");
    CHECK_VECTOR(
        v, {"8", "1", "9", "0", "8", "1", "9", "0", "1", "2", "3", "4", "5"});
    CHECK_ALIVE_COUNT(17);
  }
  CHECK_ALIVE_COUNT(13);
}

TEST_CASE(NTVector, Comparision) {
  {
    NTVector v1{"foo", "bar", "baz"};
    NTVector v2{"foo", "bar", "baz"};
    CHECK_EQ(v1, v2);
  }
  {
    NTVector v1{"foo", "bar", "baz"};
    NTVector v2{"foo", "bar", "abc"};
    CHECK_NE(v1, v2);

    v1 = {"11", "22", "33"};
    v2 = {"11", "22"};
    CHECK_NE(v1, v2);
  }
  {
    NTVector v1{"foo", "bar", "baz"};
    NTVector v2{"foo", "bar"};
    CHECK_TRUE(v1 > v2);
    CHECK_TRUE(v1 >= v2);

    CHECK_TRUE(v2 < v1);
    CHECK_TRUE(v2 <= v1);

    v1 = {"11", "22", "33"};
    v2 = {"11", "22", "33"};
    CHECK_TRUE(v1 >= v2);
    CHECK_TRUE(v2 <= v1);
  }
}

template <typename T, bool PropagateCopy = false, bool PropagateMove = false>
struct MockAllocator {
  using value_type = T;

  template <typename U>
  struct rebind {
    using other = MockAllocator<U, PropagateCopy, PropagateMove>;
  };
  using propagate_on_container_copy_assignment =
      std::bool_constant<PropagateCopy>;
  using propagate_on_container_move_assignment =
      std::bool_constant<PropagateMove>;
  int id;

  MockAllocator(int id_ = 0) : id(id_) {}

  template <typename U>
  MockAllocator(const MockAllocator<U, PropagateCopy, PropagateMove>& other)
      : id(other.id) {}

  T* allocate(std::size_t n) {
    if (n == 0)
      return nullptr;
    return static_cast<T*>(::operator new(n * sizeof(T)));
  }

  void deallocate(T* p, std::size_t n) {
    ::operator delete(p);
    AXIO_IGNORE(n);
  }

  bool operator==(const MockAllocator& other) const { return id == other.id; }
  bool operator!=(const MockAllocator& other) const { return id != other.id; }
};

TEST_CASE(NTVector, ConstructorWithCustomAllocator) {
  MockAllocator<NT> alloc(101);
  axio::Vector<NT, MockAllocator<NT>> v(alloc);
  CHECK_EQ(v.GetAllocator().id, 101);
}

TEST_CASE(NTVector, MoveConstructorSameAlloc) {
  MockAllocator<NT> alloc(10);
  axio::Vector<NT, MockAllocator<NT>> v1({"1", "2", "3"}, alloc);
  auto* original_ptr = v1.Data();
  CHECK_VECTOR(v1, {"1", "2", "3"});

  axio::Vector<NT, MockAllocator<NT>> v2(axio::Move(v1), MockAllocator<NT>(10));

  CHECK_ALIVE_COUNT(3);
  CHECK_VECTOR(v2, {"1", "2", "3"});
  CHECK_EQ(v1.Data(), nullptr);
  CHECK_EQ(v2.Data(), original_ptr);
  CHECK_EQ(v1.GetAllocator().id, v2.GetAllocator().id);
}

TEST_CASE(NTVector, MoveConstructorDifferentAlloc) {
  MockAllocator<NT> alloc1(10);
  axio::Vector<NT, MockAllocator<NT>> v1({"1", "2", "3"}, alloc1);
  auto* original_ptr = v1.Data();

  MockAllocator<NT> alloc2(20);
  axio::Vector<NT, MockAllocator<NT>> v2(axio::Move(v1), alloc2);

  CHECK_ALIVE_COUNT(3);
  CHECK_EQ(v1.GetAllocator().id, 10);
  CHECK_EQ(v2.GetAllocator().id, 20);
  CHECK_NE(v2.Data(), original_ptr);
  CHECK_VECTOR(v2, {"1", "2", "3"});
  CHECK_TRUE(v1.IsEmpty());
}

TEST_CASE(NTVector, CopyAssignmentPropagation) {
  using PropagatingAlloc = MockAllocator<NT, true>;

  axio::Vector<NT, PropagatingAlloc> v1({"1"}, PropagatingAlloc(10));
  CHECK_ALIVE_COUNT(1);
  axio::Vector<NT, PropagatingAlloc> v2({"2", "3"}, PropagatingAlloc(20));
  CHECK_ALIVE_COUNT(3);
  v2 = v1;
  CHECK_ALIVE_COUNT(2);
  CHECK_EQ(v2.GetAllocator().id, 10);
  CHECK_VECTOR(v2, {"1"});
}

TEST_CASE(NTVector, MoveAssignmentPropagation) {
  using MoveAlloc = MockAllocator<NT, false, true>;

  axio::Vector<NT, MoveAlloc> v1({"1", "2"}, MoveAlloc(10));
  CHECK_ALIVE_COUNT(2);
  CHECK_VECTOR(v1, {"1", "2"});
  auto* original_v1_ptr = v1.Data();
  axio::Vector<NT, MoveAlloc> v2({"3", "4"}, MoveAlloc(20));
  CHECK_ALIVE_COUNT(4);
  CHECK_VECTOR(v2, {"3", "4"});

  v2 = axio::Move(v1);
  CHECK_ALIVE_COUNT(2);

  CHECK_VECTOR(v2, {"1", "2"});
  CHECK_EQ(v2.GetAllocator().id, 10);
  CHECK_EQ(v2.Data(), original_v1_ptr);
  CHECK_EQ(v1.Data(), nullptr);
}

TEST_CASE(NTVector, ReservePreservesAllocator) {
  MockAllocator<NT> alloc(999);
  axio::Vector<NT, MockAllocator<NT>> v(alloc);

  v.Push("1");
  v.Reserve(24);

  CHECK_EQ(v.GetAllocator().id, 999);
  CHECK_EQ(v.Capacity(), 24);
  CHECK_VECTOR(v, {"1"});
}

TEST_CASE(NTVector, MoveAssignmentDifferentAlloc) {
  using Alloc = MockAllocator<NT, false, false>;

  Vector<NT, Alloc> v1({"1", "2", "3"}, Alloc(10));
  Vector<NT, Alloc> v2({"4", "5"}, Alloc(20));
  CHECK_ALIVE_COUNT(5);
  NT* v1_ptr = v1.Data();
  NT* v2_ptr = v2.Data();

  v2 = axio::Move(v1);
  CHECK_ALIVE_COUNT(3);

  CHECK_EQ(v2.GetAllocator().id, 20);
  CHECK_NE(v2_ptr, v1_ptr);
  CHECK_TRUE(v1.IsEmpty());
  CHECK_VECTOR(v2, {"1", "2", "3"});
}