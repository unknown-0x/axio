#ifndef AXIO_STRING_STRING_UTILS_HPP_
#define AXIO_STRING_STRING_UTILS_HPP_

#include "axio_repr.hpp"
#include "buffer.hpp"
#include "string.hpp"

#include "../container/tuple.hpp"

namespace axio {
String StringCat() {
  return String();
}

template <typename... Ts>
String StringCat(Ts&&... args) {
  static_assert(kHasAxioReprPack<Ts...>,
                "All arguments to StringCat must support AxioRepr");

  Buffer<> buffer{};
  (AxioRepr(buffer, axio::Forward<Ts>(args)), ...);
  return String(buffer.Data(), buffer.Size());
}

void StringAppend(String&) {}

template <typename... Ts>
void StringAppend(String& s, Ts&&... args) {
  static_assert(kHasAxioReprPack<Ts...>,
                "All arguments to StringAppend must support AxioRepr");
  Buffer<> buffer{};
  (AxioRepr(buffer, axio::Forward<Ts>(args)), ...);
  s.Append(buffer.Data(), buffer.Size());
}

namespace detail {
template <typename InputIt>
void StringJoin(String& output,
                InputIt first,
                InputIt last,
                std::string_view separator) {
  if (first == last) {
    return;
  }

  const char* const sep = separator.data();
  const auto size = separator.size();

  Buffer<> buffer{};
  AxioRepr(buffer, *first++);
  while (first != last) {
    buffer.Append(sep, size);
    AxioRepr(buffer, *first++);
  }
  output.Append(buffer.Data(), buffer.Size());
}

template <typename InputIt, typename Formatter>
void StringJoin(String& output,
                InputIt first,
                InputIt last,
                std::string_view separator,
                Formatter&& formatter) {
  if (first == last) {
    return;
  }

  const char* const sep = separator.data();
  const auto size = separator.size();

  Buffer<> buffer{};
  formatter(buffer, *first++);
  while (first != last) {
    buffer.Append(sep, size);
    formatter(buffer, *first++);
  }
  output.Append(buffer.Data(), buffer.Size());
}

template <typename... Ts, SizeT... Is>
void JoinRestOfTuple(Buffer<>& buffer,
                     const Tuple<Ts...>& tuple,
                     std::string_view separator,
                     std::index_sequence<Is...>) {
  const char* const sep = separator.data();
  const auto size = separator.size();

  AXIO_IGNORE(sep);
  AXIO_IGNORE(size);

  ((buffer.Append(sep, size), AxioRepr(buffer, Get<Is + 1>(tuple))), ...);
}
}  // namespace detail

template <typename Container>
String StringJoin(const Container& container, std::string_view separator) {
  String result;
  detail::StringJoin(result, std::begin(container), std::end(container),
                     separator);
  return result;
}

template <typename Container, typename Formatter>
String StringJoin(const Container& container,
                  std::string_view separator,
                  Formatter&& formatter) {
  String result;
  detail::StringJoin(result, std::begin(container), std::end(container),
                     separator, Forward<Formatter>(formatter));
  return result;
}

String StringJoin(Tuple<>, std::string_view) {
  return String();
}

template <typename... Ts>
String StringJoin(const Tuple<Ts...>& tuple, std::string_view separator) {
  Buffer<> buffer{};
  AxioRepr(buffer, Get<0>(tuple));
  detail::JoinRestOfTuple(buffer, tuple, separator,
                          std::make_index_sequence<sizeof...(Ts) - 1>{});
  return String(buffer.Data(), buffer.Size());
}

String StringJoinValues(std::string_view) {
  return String();
}

template <typename... Values>
String StringJoinValues(std::string_view separator, Values&&... values) {
  const char* const sep = separator.data();
  const auto size = separator.size();

  Buffer<> buffer{};
  SizeT index = 0;
  (((index++ > 0 ? (void)(buffer.Append(sep, size)) : (void)0),
    AxioRepr(buffer, Forward<Values>(values))),
   ...);

  return String(buffer.Data(), buffer.Size());
}

struct CharDelimiter {
  char delim;

  const char* Find(const char* first, const char* last) const {
    if (first == last) {
      return last;
    }
    const auto* p = std::memchr(first, delim, static_cast<SizeT>(last - first));
    return p ? static_cast<const char*>(p) : last;
  }

  SizeT Size() const noexcept { return 1; }
};

struct StringDelimiter {
  const char* delim;
  SizeT size;

  explicit StringDelimiter(std::string_view view)
      : delim(view.data()), size(view.size()) {}

  const char* Find(const char* first, const char* last) const {
    const auto remaining = static_cast<SizeT>(last - first);
    if (size > remaining) {
      return last;
    }

    for (; first != last; ++first) {
      if (std::memcmp(first, delim, size) == 0) {
        return first;
      }
    }
    return last;
  }

  SizeT Size() const noexcept { return size; }
};

struct AnyCharDelimiter {
  const char* delim;
  SizeT size;

  explicit AnyCharDelimiter(std::string_view view)
      : delim(view.data()), size(view.size()) {}

  const char* Find(const char* first, const char* last) const {
    for (; first != last; ++first) {
      if (std::memchr(delim, *first, size)) {
        return first;
      }
    }
    return last;
  }

  SizeT Size() const noexcept { return 1; }
};

template <typename Delimiter>
class SplitView {
 public:
  struct Iterator {
    using iterator_category = std::input_iterator_tag;
    using value_type = std::string_view;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type;

    Iterator() noexcept : current_(nullptr), end_(nullptr), last_(nullptr) {}

    Iterator(const char* current, const char* end, Delimiter* delim)
        : current_(current), end_(end), last_(nullptr), delim_(delim) {
      Advance();
    }

    std::string_view operator*() const {
      return std::string_view{last_, static_cast<SizeT>(current_ - last_)};
    }

    Iterator& operator++() {
      Advance();
      return *this;
    }

    Iterator operator++(int) {
      Iterator temp = *this;
      Advance();
      return temp;
    }

    Bool operator==(const Iterator& other) const noexcept {
      return current_ == other.current_;
    }

    Bool operator!=(const Iterator& other) const noexcept {
      return current_ != other.current_;
    }

   private:
    void Advance() {
      if (current_ == nullptr) {
        return;
      }

      if (last_ == nullptr) {
        last_ = current_;
        current_ = delim_->Find(current_, end_);
      } else {
        if (current_ == end_) {
          current_ = nullptr;
          end_ = nullptr;
          last_ = nullptr;
        } else {
          current_ += delim_->Size();
          last_ = current_;
          current_ = delim_->Find(current_, end_);
        }
      }
    }

    const char* current_;
    const char* end_;
    const char* last_;
    Delimiter* delim_;
  };

  SplitView(std::string_view str, Delimiter delim) : str_(str), delim_(delim) {}

  Iterator begin() {
    const auto* current = str_.data();
    return Iterator{current, current + str_.size(), &delim_};
  }

  Iterator end() { return Iterator(); }

 private:
  std::string_view str_;
  Delimiter delim_;
};

inline auto Split(std::string_view sv, char delim) {
  return SplitView<CharDelimiter>{sv, CharDelimiter{delim}};
}

template <std::size_t N>
inline auto Split(std::string_view sv, const char (&delim)[N]) {
  return SplitView<StringDelimiter>{
      sv, StringDelimiter{std::string_view(delim, N - 1)}};
}

inline auto Split(std::string_view sv, const char* delim) {
  return SplitView<StringDelimiter>{sv,
                                    StringDelimiter{std::string_view(delim)}};
}

inline auto Split(std::string_view sv, std::string_view delim) {
  return SplitView<StringDelimiter>{sv, StringDelimiter{delim}};
}

template <typename Delimiter>
inline auto Split(std::string_view sv, Delimiter delim) {
  return SplitView<Delimiter>{sv, delim};
}

template <typename Derived, typename BaseIterator>
struct ViewIteratorBase {
  using iterator_category = std::input_iterator_tag;
  using value_type = std::string_view;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type*;
  using reference = value_type;

  BaseIterator base_it_;

  Derived operator++(int) {
    Derived temp = *static_cast<Derived*>(this);
    ++(*static_cast<Derived*>(this));
    return temp;
  }

  Bool operator!=(const ViewIteratorBase& other) const {
    return !(static_cast<const Derived&>(*this) ==
             static_cast<const Derived&>(other));
  }
};

template <typename UnderlyingView>
struct TrimView {
 public:
  struct Iterator
      : public ViewIteratorBase<Iterator, typename UnderlyingView::Iterator> {
    std::string_view operator*() const {
      std::string_view s = *(this->base_it_);
      if (s.empty()) {
        return s;
      }

      SizeT start = 0;
      SizeT end = s.size();
      while (start < s.size() &&
             std::isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
      }

      if (start == end) {
        return {};
      }

      while (end > start &&
             std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
      }

      return s.substr(start, end - start);
    }

    Iterator& operator++() {
      ++(this->base_it_);
      return *this;
    }

    Bool operator==(const Iterator& other) const {
      return this->base_it_ == other.base_it_;
    }
  };

  explicit TrimView(UnderlyingView view) : view_(view) {}
  Iterator begin() { return Iterator{{view_.begin()}}; }
  Iterator end() { return Iterator{{view_.end()}}; }

 private:
  UnderlyingView view_;
};

struct TrimAdapter {};
inline constexpr TrimAdapter Trim;

template <typename UnderlyingView>
inline auto operator|(UnderlyingView view, TrimAdapter) {
  return TrimView<UnderlyingView>(view);
}

template <typename UnderlyingView, typename Predicate>
struct FilterView {
  struct Iterator
      : public ViewIteratorBase<Iterator, typename UnderlyingView::Iterator> {
    typename UnderlyingView::Iterator base_end_;
    Predicate pred_;

    std::string_view operator*() const { return *(this->base_it_); }

    Iterator& operator++() {
      ++(this->base_it_);
      AdvanceToValid();
      return *this;
    }

    Bool operator==(const Iterator& other) const {
      return this->base_it_ == other.base_it_;
    }

    void AdvanceToValid() {
      while (this->base_it_ != base_end_ && !pred_(*(this->base_it_))) {
        ++(this->base_it_);
      }
    }
  };

  FilterView(UnderlyingView view, Predicate pred)
      : view_(axio::Move(view)), pred_(pred) {}

  Iterator begin() {
    auto it = Iterator{{view_.begin()}, {view_.end()}, pred_};
    it.AdvanceToValid();
    return it;
  }

  Iterator end() { return Iterator{{view_.end()}, {view_.end()}, pred_}; }

 private:
  UnderlyingView view_;
  Predicate pred_;
};

template <typename Predicate>
struct FilterAdapter {
  Predicate pred;
};

template <typename Predicate>
inline auto Filter(Predicate&& pred) {
  using DecayedPred = typename Decay<Predicate>::type;
  return FilterAdapter<DecayedPred>{axio::Forward<Predicate>(pred)};
}

template <typename UnderlyingView, typename Predicate>
inline auto operator|(UnderlyingView view, FilterAdapter<Predicate> adaptor) {
  return FilterView<UnderlyingView, Predicate>(axio::Move(view),
                                               axio::Move(adaptor.pred));
}

struct SkipEmptyAdapter {};
inline constexpr SkipEmptyAdapter SkipEmpty;

template <typename UnderlyingView>
inline auto operator|(UnderlyingView view, SkipEmptyAdapter) {
  return FilterView(axio::Move(view),
                    [](std::string_view s) noexcept { return !s.empty(); });
}

template <typename UnderlyingView>
struct TakeView {
  using UnderlyingIterator = decltype(std::declval<UnderlyingView>().begin());

  struct Iterator
      : public ViewIteratorBase<Iterator, typename UnderlyingView::Iterator> {
    using Base = ViewIteratorBase<Iterator, typename UnderlyingView::Iterator>;

    Iterator(typename UnderlyingView::Iterator base_it, SizeT n)
        : Base{base_it}, count_(n) {}

    std::string_view operator*() const { return *(this->base_it_); }

    Iterator& operator++() {
      ++this->base_it_;
      if (count_ > 0) {
        --count_;
      }
      return *this;
    }

    Bool operator==(const Iterator& other) const {
      if (count_ == other.count_) {
        return true;
      }
      return this->base_it_ == other.base_it_;
    }

   private:
    SizeT count_;
  };

  TakeView(UnderlyingView view, SizeT n) : view_(axio::Move(view)), n_(n) {}
  auto begin() { return Iterator(view_.begin(), n_); }
  auto end() { return Iterator(view_.end(), 0); }

 private:
  UnderlyingView view_;
  SizeT n_;
};

struct TakeAdapter {
  SizeT n;
};

inline auto Take(SizeT n) {
  return TakeAdapter{n};
}

template <typename UnderlyingView>
inline auto operator|(UnderlyingView&& view, TakeAdapter adapter) {
  return TakeView<typename Decay<UnderlyingView>::type>(
      axio::Forward<UnderlyingView>(view), adapter.n);
}

template <typename UnderlyingView>
struct DropView {
  using BaseIteratorType = decltype(std::declval<UnderlyingView>().begin());

  struct Iterator : public ViewIteratorBase<Iterator, BaseIteratorType> {
    using Base = ViewIteratorBase<Iterator, BaseIteratorType>;

    Iterator(BaseIteratorType base_it) : Base{base_it} {}

    std::string_view operator*() const { return *(this->base_it_); }

    Iterator& operator++() {
      ++this->base_it_;
      return *this;
    }

    bool operator==(const Iterator& other) const {
      return this->base_it_ == other.base_it_;
    }
  };

  DropView(UnderlyingView view, SizeT n) : view_(axio::Move(view)), n_(n) {}

  auto begin() {
    auto it = view_.begin();
    auto end_it = view_.end();
    for (SizeT i = 0; i < n_ && it != end_it; ++i) {
      ++it;
    }

    return Iterator(it);
  }

  auto end() { return Iterator(view_.end()); }

 private:
  UnderlyingView view_;
  SizeT n_;
};

struct DropAdapter {
  SizeT n;
};

inline auto Drop(SizeT n) {
  return DropAdapter{n};
}

template <typename UnderlyingView>
inline auto operator|(UnderlyingView&& view, DropAdapter adapter) {
  return DropView<typename Decay<UnderlyingView>::type>(
      axio::Forward<UnderlyingView>(view), adapter.n);
}

namespace detail {
template <typename T, typename = void>
struct ExtractValueType {
  using type = typename T::value_type;
};

template <typename T>
struct ExtractValueType<T, Void<typename T::ValueType>> {
  using type = typename T::ValueType;
};

template <typename T, typename V>
using push_back_op = decltype(std::declval<T>().push_back(std::declval<V>()));

template <typename T, typename V>
using Push_Op = decltype(std::declval<T>().Push(std::declval<V>()));
}  // namespace detail
template <typename Container>
struct ToContainerAdaptor {};

template <typename Container>
inline auto To() {
  return ToContainerAdaptor<Container>{};
}

template <typename UnderlyingView, typename Container>
inline auto operator|(UnderlyingView&& view, ToContainerAdaptor<Container>) {
  using ValueType = typename detail::ExtractValueType<Container>::type;

  Container container;

  for (auto token : view) {
    if constexpr (IsDetected<detail::push_back_op, Container,
                             ValueType>::value) {
      container.push_back(ValueType(token));
    } else if constexpr (IsDetected<detail::Push_Op, Container,
                                    ValueType>::value) {
      container.Push(ValueType(token));
    } else {
      auto it = std::inserter(container, container.end());
      *it = ValueType(token);
    }
  }

  return container;
}
}  // namespace axio

#endif