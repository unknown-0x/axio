#include <simpletest/simpletest.hpp>

#include <axio/container/array.hpp>
#include <axio/container/tuple.hpp>
#include <axio/container/vector.hpp>
#include <axio/string/axio_repr.hpp>
#include <axio/string/string.hpp>

using namespace axio;

#define CHECK_REPR(s, value, expected) \
  s.Clear();                           \
  AxioRepr(s, value);                  \
  CHECK_EQ(s, expected);

TEST_CASE(AxioRepr, Integer) {
  axio::String s;
  CHECK_REPR(s, UInt8(0), "0");
  CHECK_REPR(s, UInt8(10), "10");
  CHECK_REPR(s, UInt8(127), "127");

  CHECK_REPR(s, Int8(-4), "-4");
  CHECK_REPR(s, Int8(-10), "-10");
  CHECK_REPR(s, Int8(-127), "-127");

  CHECK_REPR(s, UInt16(0), "0");
  CHECK_REPR(s, UInt16(100), "100");
  CHECK_REPR(s, UInt16(60507), "60507");

  CHECK_REPR(s, Int16(-42), "-42");
  CHECK_REPR(s, Int16(-1040), "-1040");
  CHECK_REPR(s, Int16(-32768), "-32768");

  CHECK_REPR(s, UInt32(320), "320");
  CHECK_REPR(s, UInt32(10423), "10423");
  CHECK_REPR(s, UInt32(55123427), "55123427");
  CHECK_REPR(s, UInt32(821512357), "821512357");

  CHECK_REPR(s, -320, "-320");
  CHECK_REPR(s, -10423, "-10423");
  CHECK_REPR(s, -55123427, "-55123427");
  CHECK_REPR(s, -821512357, "-821512357");

  CHECK_REPR(s, UInt64(320), "320");
  CHECK_REPR(s, UInt64(10423), "10423");
  CHECK_REPR(s, UInt64(55123427), "55123427");
  CHECK_REPR(s, UInt64(821512357), "821512357");
  CHECK_REPR(s, UInt64(32712342342), "32712342342");
  CHECK_REPR(s, UInt64(56122712342342), "56122712342342");
  CHECK_REPR(s, UInt64(1523332712342342), "1523332712342342");

  CHECK_REPR(s, Int64(-320), "-320");
  CHECK_REPR(s, Int64(-10423), "-10423");
  CHECK_REPR(s, Int64(-55123427), "-55123427");
  CHECK_REPR(s, Int64(-821512357), "-821512357");
  CHECK_REPR(s, -32712342342, "-32712342342");
  CHECK_REPR(s, -56122712342342, "-56122712342342");
  CHECK_REPR(s, -1523332712342342, "-1523332712342342");
}

#define CHECK_REPR_FLOAT(s, value, expected) \
  s.Clear();                                 \
  AxioRepr(s, value);                        \
  CHECK_TRUE(s.Contains(expected))

TEST_CASE(AxioRepr, FloatAndDouble) {
  axio::String s;
  CHECK_REPR_FLOAT(s, 3.14159F, "3.14159");
  CHECK_REPR_FLOAT(s, (6.62607e-34f), "6.62607e-34");
  CHECK_REPR_FLOAT(s, (1.342178e+08f), "1.342178e+08");
  CHECK_REPR_FLOAT(s, (1.3421781e+08f), "1.3421781e+08");

  CHECK_REPR_FLOAT(s, 3.14159265358979, "3.14159265358979");
  CHECK_REPR_FLOAT(s, 1.3588129002659584e-245, "1.3588129002659584e-245");
  CHECK_REPR_FLOAT(s, 2.9802322387695312e-08, "2.9802322387695312e-08");
  CHECK_REPR_FLOAT(s, 5.960464477539063e-08, "5.960464477539063e-08");
  CHECK_REPR_FLOAT(s, 1.3076622631878654e+65, "1.3076622631878654e+65");
  CHECK_REPR_FLOAT(s, 9.03725590277404e+159, "9.03725590277404e+159");
  CHECK_REPR_FLOAT(s, 9.03725590277404e+160, "9.03725590277404e+160");
  CHECK_REPR_FLOAT(s, 9.03725590277404e+161, "9.03725590277404e+161");
  CHECK_REPR_FLOAT(s, 9.03725590277404e+162, "9.03725590277404e+162");
  CHECK_REPR_FLOAT(s, 5.960464477539062e-07, "5.960464477539062e-07");
  CHECK_REPR_FLOAT(s, (43210.0), "43210");
  CHECK_REPR_FLOAT(s, (43210.1), "43210.1");
  CHECK_REPR_FLOAT(s, (10000), "10000");
  CHECK_REPR_FLOAT(s, (-5942736479622170.0), "-5942736479622170");

  CHECK_REPR_FLOAT(s, (std::numeric_limits<double>::denorm_min()), "5e-324");
  CHECK_REPR_FLOAT(s, (1e-323), "1e-323");
  CHECK_REPR_FLOAT(s, (1.2e-322), "1.2e-322");
  CHECK_REPR_FLOAT(s, (1.5e-323), "1.5e-323");
  CHECK_REPR_FLOAT(s, (1.24e-322), "1.24e-322");
  CHECK_REPR_FLOAT(s, (1.234e-320), "1.234e-320");
  CHECK_REPR_FLOAT(s, (2.2250738585072004e-308), "2.2250738585072004e-308");
}

TEST_CASE(AxioRepr, Bool) {
  String s;
  CHECK_REPR(s, true, "true");
  CHECK_REPR(s, false, "false");

  const int x = 10;
  const int y = 42;
  CHECK_REPR(s, x < y, "true");
  CHECK_REPR(s, x > y, "false");
}

TEST_CASE(AxioRepr, String) {
  String s;

  char arr[]{"this is my string!!! hello world"};
  CHECK_REPR(s, arr, "this is my string!!! hello world");

  std::string_view sv{"string view"};
  CHECK_REPR(s, sv, "string view");

  const char* cc = "const char*";
  CHECK_REPR(s, cc, "const char*");

  char* arr_ptr = arr;
  CHECK_REPR(s, arr_ptr, "this is my string!!! hello world");

  CHECK_REPR(s, 'a', "a");
  CHECK_REPR(s, '\n', "\n");
  CHECK_REPR(s, '%', "%");

  String foo{"Foo"};
  CHECK_REPR(s, foo, "Foo");
}

TEST_CASE(AxioRepr, Vector) {
  String s;

  Vector<int> v1{};
  Vector<int> v2{1};
  Vector<int> v3{1, 2};
  Vector<int> v4{1, 2, 3};
  Vector<int> v5{1, 2, 3, 4};
  Vector<int> v6{1, 2, 3, 4, 5, 6, 7};

  CHECK_REPR(s, v1, "[]");
  CHECK_REPR(s, v2, "[1]");
  CHECK_REPR(s, v3, "[1, 2]");
  CHECK_REPR(s, v4, "[1, 2, 3]");
  CHECK_REPR(s, v5, "[1, 2, 3, 4]");
  CHECK_REPR(s, v6, "[1, 2, 3, 4, 5, 6, 7]");

  Vector<Vector<String>> complex_v{
      {"foo", "bar"}, {"aaaa", "bbb", "asdfghj"}, {"1", "34", "!&#$%#>?"}};
  CHECK_REPR(s, complex_v,
             "[[foo, bar], [aaaa, bbb, asdfghj], [1, 34, !&#$%#>?]]");
}

struct Foo {};

template <typename Output>
void AxioRepr(Output& output, Foo) {
  output.Append("Foo");
}

TEST_CASE(AxioRepr, Tuple) {
  String s;

  Tuple<> t1;
  Tuple<int> t2{42};
  Tuple<int, int, int> t3{1, 2, 3};
  Tuple<int, int, String, Vector<Foo>> t4{1, 2, "hello", {{}, {}}};

  CHECK_REPR(s, t1, "()");
  CHECK_REPR(s, t2, "(42)");
  CHECK_REPR(s, t3, "(1, 2, 3)");
  CHECK_REPR(s, t4, "(1, 2, hello, [Foo, Foo])");
}

TEST_CASE(AxioRepr, Array) {
  String s;

  Array<int, 0> a1;
  Array<int, 3> a2{1, 2, 3};
  Array<Tuple<int, String, Foo>, 3> a3{Tuple<int, String, Foo>{1, "foo", {}},
                                       Tuple<int, String, Foo>{2, "bar", {}},
                                       Tuple<int, String, Foo>{3, "baz", {}}};

  CHECK_REPR(s, a1, "[]");
  CHECK_REPR(s, a2, "[1, 2, 3]");
  CHECK_REPR(s, a3, "[(1, foo, Foo), (2, bar, Foo), (3, baz, Foo)]");
}