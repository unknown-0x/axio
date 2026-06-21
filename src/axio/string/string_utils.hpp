/**
 * @file string_utils.hpp
 * @brief String-building helpers (StringCat, StringAppend, StringJoin) and
 *        a small `std::string_view`-based "ranges-lite" toolkit for
 *        splitting and lazily transforming strings (Split, Trim, Filter,
 *        SkipEmpty, Take, Drop, To).
 */

#ifndef AXIO_STRING_STRING_UTILS_HPP_
#define AXIO_STRING_STRING_UTILS_HPP_

#include "axio_repr.hpp"
#include "buffer.hpp"
#include "string.hpp"

#include "../container/tuple.hpp"

namespace axio {
/// @return An empty String. Base case for the variadic StringCat overload.
String StringCat() {
  return String();
}

/**
 * @brief Formats each argument via AxioRepr() and concatenates the
 *        results into a newly constructed String.
 *
 * @tparam Ts Argument types; each must satisfy HasAxioRepr.
 * @param args Values to format, in order.
 * @return A new String containing the concatenated textual
 *         representation of @p args.
 */
template <typename... Ts>
String StringCat(Ts&&... args) {
  static_assert((HasAxioRepr<Ts>::value && ...),
                "All arguments to StringCat must support AxioRepr");

  Buffer<> buffer{};
  (AxioRepr(buffer, axio::Forward<Ts>(args)), ...);
  return String(buffer.Data(), buffer.Size());
}

/// No-op base case for the variadic StringAppend overload.
void StringAppend(String&) {}

/**
 * @brief Formats each argument via AxioRepr() and appends the results to
 *        an existing String.
 *
 * @tparam Ts Argument types; each must satisfy HasAxioRepr.
 * @param s    String to append to.
 * @param args Values to format, in order.
 */
template <typename... Ts>
void StringAppend(String& s, Ts&&... args) {
  static_assert((HasAxioRepr<Ts>::value && ...),
                "All arguments to StringAppend must support AxioRepr");
  Buffer<> buffer{};
  (AxioRepr(buffer, axio::Forward<Ts>(args)), ...);
  s.Append(buffer.Data(), buffer.Size());
}

namespace detail {
/**
 * @brief Formats the elements `[first, last)` via AxioRepr(), joining
 *        consecutive elements with @p separator, and appends the result
 *        to @p output.
 *
 * Does nothing if `first == last`.
 *
 * @tparam InputIt Input iterator type whose value type satisfies
 *                 HasAxioRepr.
 * @param output    String to append the joined result to.
 * @param first     Iterator to the first element.
 * @param last      Iterator one past the last element.
 * @param separator Text inserted between consecutive elements.
 */
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

/**
 * @brief Like the two-argument StringJoin(), but formats each element
 *        with a caller-supplied @p formatter instead of AxioRepr().
 *
 * @tparam InputIt   Input iterator type.
 * @tparam Formatter Callable with signature `void(Buffer<>&, decltype(*it))`.
 * @param output    String to append the joined result to.
 * @param first     Iterator to the first element.
 * @param last      Iterator one past the last element.
 * @param separator Text inserted between consecutive elements.
 * @param formatter Callable invoked once per element to write its text
 *                  into the internal scratch buffer.
 */
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

/**
 * @brief Appends elements 1..N of @p tuple (i.e. all but the first,
 *        which the caller is expected to have already written) to
 *        @p buffer, each preceded by @p separator.
 *
 * @tparam Ts Tuple element types.
 * @tparam Is Index pack identifying elements 1..N relative to element 0;
 *            expand via `std::make_index_sequence<sizeof...(Ts) - 1>`.
 * @param buffer    Scratch buffer accumulating the formatted text.
 * @param tuple      Tuple whose remaining elements are appended.
 * @param separator Text inserted before each appended element.
 */
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

/**
 * @brief Joins the elements of @p container, formatted via AxioRepr(),
 *        separated by @p separator.
 * @tparam Container Range type usable with `std::begin`/`std::end` whose
 *                    value type satisfies HasAxioRepr.
 * @param container Elements to join.
 * @param separator Text inserted between consecutive elements.
 * @return A new String containing the joined result, or an empty String
 *         if @p container is empty.
 */
template <typename Container>
String StringJoin(const Container& container, std::string_view separator) {
  String result;
  detail::StringJoin(result, std::begin(container), std::end(container),
                     separator);
  return result;
}

/**
 * @brief Joins the elements of @p container using a caller-supplied
 *        @p formatter instead of AxioRepr(), separated by @p separator.
 * @tparam Container Range type usable with `std::begin`/`std::end`.
 * @tparam Formatter Callable with signature `void(Buffer<>&, decltype(*it))`.
 * @param container Elements to join.
 * @param separator Text inserted between consecutive elements.
 * @param formatter Callable invoked once per element to format it.
 * @return A new String containing the joined result, or an empty String
 *         if @p container is empty.
 */
template <typename Container, typename Formatter>
String StringJoin(const Container& container,
                  std::string_view separator,
                  Formatter&& formatter) {
  String result;
  detail::StringJoin(result, std::begin(container), std::end(container),
                     separator, Forward<Formatter>(formatter));
  return result;
}

/// @return An empty String. Overload for joining an empty Tuple<>.
String StringJoin(Tuple<>, std::string_view) {
  return String();
}

/**
 * @brief Joins the (heterogeneous) elements of @p tuple, each formatted
 *        via AxioRepr(), separated by @p separator.
 * @tparam Ts Tuple element types; each must satisfy HasAxioRepr.
 * @param tuple     Tuple whose elements are joined, in order.
 * @param separator Text inserted between consecutive elements.
 * @return A new String containing the joined result.
 */
template <typename... Ts>
String StringJoin(const Tuple<Ts...>& tuple, std::string_view separator) {
  Buffer<> buffer{};
  AxioRepr(buffer, Get<0>(tuple));
  detail::JoinRestOfTuple(buffer, tuple, separator,
                          std::make_index_sequence<sizeof...(Ts) - 1>{});
  return String(buffer.Data(), buffer.Size());
}

/// @return An empty String. Base case for the variadic StringJoinValues
///         overload.
String StringJoinValues(std::string_view) {
  return String();
}

/**
 * @brief Formats each of @p values via AxioRepr() and joins the results
 *        with @p separator, without requiring them to share a type or be
 *        stored in a container.
 * @tparam Values Argument types; each must satisfy HasAxioRepr.
 * @param separator Text inserted between consecutive values.
 * @param values    Values to format and join, in order.
 * @return A new String containing the joined result.
 */
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

/**
 * @brief Delimiter strategy that splits on a single character, located
 *        via `std::memchr`. Used as the Delimiter type parameter of
 *        SplitView.
 */
struct CharDelimiter {
  char delim;

  /**
   * @brief Finds the next occurrence of #delim within `[first, last)`.
   * @param first Start of the search range.
   * @param last  End of the search range (exclusive).
   * @return Pointer to the matched character, or @p last if not found.
   */
  const char* Find(const char* first, const char* last) const {
    if (first == last) {
      return last;
    }
    const auto* p = std::memchr(first, delim, static_cast<SizeT>(last - first));
    return p ? static_cast<const char*>(p) : last;
  }

  /// @return The width of the delimiter in characters (always 1).
  SizeT Size() const noexcept { return 1; }
};

/**
 * @brief Delimiter strategy that splits on a fixed substring, matched via
 *        `std::memcmp`. Used as the Delimiter type parameter of
 *        SplitView.
 */
struct StringDelimiter {
  const char* delim;
  SizeT size;

  /**
   * @brief Constructs a delimiter referencing @p view's character data.
   * @param view Delimiter text; must outlive this object.
   */
  explicit StringDelimiter(std::string_view view)
      : delim(view.data()), size(view.size()) {}

  /**
   * @brief Finds the next occurrence of the delimiter substring within
   *        `[first, last)`.
   * @param first Start of the search range.
   * @param last  End of the search range (exclusive).
   * @return Pointer to the start of the match, or @p last if the
   *         delimiter does not occur (or does not fit) in the range.
   */
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

  /// @return The width of the delimiter in characters.
  SizeT Size() const noexcept { return size; }
};

/**
 * @brief Delimiter strategy that splits on any one of a set of
 *        characters (analogous to `strpbrk`). Used as the Delimiter type
 *        parameter of SplitView.
 */
struct AnyCharDelimiter {
  const char* delim;
  SizeT size;

  /**
   * @brief Constructs a delimiter referencing @p view's character data
   *        as the set of characters to split on.
   * @param view Candidate delimiter characters; must outlive this
   *             object.
   */
  explicit AnyCharDelimiter(std::string_view view)
      : delim(view.data()), size(view.size()) {}

  /**
   * @brief Finds the next character within `[first, last)` that belongs
   *        to the delimiter set.
   * @param first Start of the search range.
   * @param last  End of the search range (exclusive).
   * @return Pointer to the matched character, or @p last if none of the
   *         characters in the range belong to the set.
   */
  const char* Find(const char* first, const char* last) const {
    for (; first != last; ++first) {
      if (std::memchr(delim, *first, size)) {
        return first;
      }
    }
    return last;
  }

  /// @return The match width in characters (always 1; only one character
  ///         is consumed per match).
  SizeT Size() const noexcept { return 1; }
};

/**
 * @brief A lazy, allocation-free view over the tokens produced by
 *        splitting a `std::string_view` using a pluggable @p Delimiter
 *        strategy.
 *
 * Models an input range: iterating it yields successive `std::string_view`
 * tokens, including empty tokens between adjacent delimiters. Use the
 * Split() factory functions to construct one rather than naming this
 * class directly.
 *
 * @tparam Delimiter Delimiter strategy type providing
 *                   `const char* Find(const char*, const char*) const`
 *                   and `SizeT Size() const`. See CharDelimiter,
 *                   StringDelimiter and AnyCharDelimiter.
 */
template <typename Delimiter>
class SplitView {
 public:
  /**
   * @brief Single-pass input iterator over the tokens of a SplitView.
   *
   * Dereferencing yields the current token as a `std::string_view` into
   * the original string (no copies are made). A default-constructed
   * Iterator represents the end of any sequence.
   */
  struct Iterator {
    using iterator_category = std::input_iterator_tag;
    using value_type = std::string_view;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type;

    /// Constructs a past-the-end (sentinel) iterator.
    Iterator() noexcept : current_(nullptr), end_(nullptr), last_(nullptr) {}

    /**
     * @brief Constructs an iterator positioned at the first token of the
     *        range `[current, end)`.
     * @param current Start of the string to split.
     * @param end     End of the string to split.
     * @param delim   Delimiter strategy used to locate token boundaries;
     *                must outlive the iterator.
     */
    Iterator(const char* current, const char* end, Delimiter* delim)
        : current_(current), end_(end), last_(nullptr), delim_(delim) {
      Advance();
    }

    /// @return The current token as a view into the original string.
    std::string_view operator*() const {
      return std::string_view{last_, static_cast<SizeT>(current_ - last_)};
    }

    /// Advances to the next token. @return Reference to `*this`.
    Iterator& operator++() {
      Advance();
      return *this;
    }

    /// Advances to the next token. @return A copy of the prior position.
    Iterator operator++(int) {
      Iterator temp = *this;
      Advance();
      return temp;
    }

    /// @return Whether both iterators are at the same position (or both
    ///         past-the-end).
    Bool operator==(const Iterator& other) const noexcept {
      return current_ == other.current_;
    }

    /// @return The negation of operator==.
    Bool operator!=(const Iterator& other) const noexcept {
      return current_ != other.current_;
    }

   private:
    /**
     * @brief Moves the iterator to the next token boundary, or to the
     *        past-the-end state if the previous token was the last one.
     */
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

    /// Position of the next delimiter (or `end_` if none remains).
    const char* current_;
    /// End of the string being split.
    const char* end_;
    /// Start of the current token.
    const char* last_;
    /// Delimiter strategy used to locate boundaries (not owned).
    Delimiter* delim_;
  };

  /**
   * @brief Constructs a view over the tokens of @p str as split by
   *        @p delim.
   * @param str   String to split. The view does not own its data, so
   *              @p str's backing storage must outlive the SplitView.
   * @param delim Delimiter strategy to use.
   */
  SplitView(std::string_view str, Delimiter delim) : str_(str), delim_(delim) {}

  /// @return An iterator positioned at the first token.
  Iterator begin() {
    const auto* current = str_.data();
    return Iterator{current, current + str_.size(), &delim_};
  }

  /// @return The past-the-end sentinel iterator.
  Iterator end() { return Iterator(); }

 private:
  /// The string being split.
  std::string_view str_;
  /// The delimiter strategy instance referenced by iterators.
  Delimiter delim_;
};

/**
 * @brief Splits @p sv on occurrences of the single character @p delim.
 * @param sv    String to split.
 * @param delim Character to split on.
 * @return A SplitView yielding tokens between (and around) occurrences of
 *         @p delim.
 */
inline auto Split(std::string_view sv, char delim) {
  return SplitView<CharDelimiter>{sv, CharDelimiter{delim}};
}

/**
 * @brief Splits @p sv on occurrences of the string literal @p delim.
 * @tparam N Length of the literal, including its null terminator
 *           (deduced).
 * @param sv    String to split.
 * @param delim Null-terminated string literal delimiter.
 * @return A SplitView yielding tokens between (and around) occurrences of
 *         @p delim.
 */
template <std::size_t N>
inline auto Split(std::string_view sv, const char (&delim)[N]) {
  return SplitView<StringDelimiter>{
      sv, StringDelimiter{std::string_view(delim, N - 1)}};
}

/**
 * @brief Splits @p sv on occurrences of the null-terminated string
 *        @p delim.
 * @param sv    String to split.
 * @param delim Null-terminated delimiter string.
 * @return A SplitView yielding tokens between (and around) occurrences of
 *         @p delim.
 */
inline auto Split(std::string_view sv, const char* delim) {
  return SplitView<StringDelimiter>{sv,
                                    StringDelimiter{std::string_view(delim)}};
}

/**
 * @brief Splits @p sv on occurrences of @p delim.
 * @param sv    String to split.
 * @param delim Delimiter string.
 * @return A SplitView yielding tokens between (and around) occurrences of
 *         @p delim.
 */
inline auto Split(std::string_view sv, std::string_view delim) {
  return SplitView<StringDelimiter>{sv, StringDelimiter{delim}};
}

/**
 * @brief Splits @p sv using a caller-supplied delimiter strategy.
 * @tparam Delimiter Delimiter strategy type; see SplitView for the
 *                   required interface.
 * @param sv    String to split.
 * @param delim Delimiter strategy instance.
 * @return A SplitView yielding tokens as determined by @p delim.
 */
template <typename Delimiter>
inline auto Split(std::string_view sv, Delimiter delim) {
  return SplitView<Delimiter>{sv, delim};
}

/**
 * @brief CRTP base implementing the boilerplate `operator++(int)` and
 *        `operator!=` shared by the lazy view iterators in this header
 *        (TrimView, FilterView, TakeView, DropView).
 *
 * Derived iterators need only provide `operator*`, `operator++()` and
 * `operator==`.
 *
 * @tparam Derived      The concrete iterator type inheriting from this
 *                      base.
 * @tparam BaseIterator Iterator type of the underlying view being
 *                      adapted.
 */
template <typename Derived, typename BaseIterator>
struct ViewIteratorBase {
  using iterator_category = std::input_iterator_tag;
  using value_type = std::string_view;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type*;
  using reference = value_type;

  /// Underlying iterator being wrapped/adapted.
  BaseIterator base_it_;

  /// Post-increment, implemented in terms of the derived type's
  /// pre-increment. @return A copy of the prior position.
  Derived operator++(int) {
    Derived temp = *static_cast<Derived*>(this);
    ++(*static_cast<Derived*>(this));
    return temp;
  }

  /// @return The negation of the derived type's `operator==`.
  Bool operator!=(const ViewIteratorBase& other) const {
    return !(static_cast<const Derived&>(*this) ==
             static_cast<const Derived&>(other));
  }
};

/**
 * @brief Lazily adapts an underlying view of `std::string_view` tokens so
 *        that each token has its leading and trailing whitespace
 *        stripped on access.
 *
 * Constructed via `view | Trim`. Whitespace is determined with
 * `std::isspace`.
 *
 * @tparam UnderlyingView Range type yielding `std::string_view` tokens
 *                        (e.g. a SplitView).
 */
template <typename UnderlyingView>
struct TrimView {
 public:
  /// Iterator that trims whitespace from each token at dereference time.
  struct Iterator
      : public ViewIteratorBase<Iterator, typename UnderlyingView::Iterator> {
    /// @return The current token with leading/trailing whitespace
    ///         removed.
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

    /// Advances to the next underlying token. @return Reference to
    /// `*this`.
    Iterator& operator++() {
      ++(this->base_it_);
      return *this;
    }

    /// @return Whether both iterators wrap equal underlying positions.
    Bool operator==(const Iterator& other) const {
      return this->base_it_ == other.base_it_;
    }
  };

  /**
   * @brief Constructs a trimming adapter over @p view.
   * @param view Underlying view of tokens to trim.
   */
  explicit TrimView(UnderlyingView view) : view_(view) {}
  /// @return An iterator at the first (trimmed) token.
  Iterator begin() { return Iterator{{view_.begin()}}; }
  /// @return The past-the-end iterator.
  Iterator end() { return Iterator{{view_.end()}}; }

 private:
  /// Underlying view being adapted.
  UnderlyingView view_;
};

/// Tag type used with `operator|` to build a TrimView. See #Trim.
struct TrimAdapter {};
/// Pipe this onto a view of tokens to lazily trim whitespace from each
/// one, e.g. `axio::Split(s, ',') | axio::Trim`.
inline constexpr TrimAdapter Trim;

/**
 * @brief Pipe operator that wraps @p view in a TrimView.
 * @tparam UnderlyingView Range type yielding `std::string_view` tokens.
 * @param view Underlying view to adapt.
 * @return A TrimView over @p view.
 */
template <typename UnderlyingView>
inline auto operator|(UnderlyingView view, TrimAdapter) {
  return TrimView<UnderlyingView>(view);
}

/**
 * @brief Lazily adapts an underlying view of `std::string_view` tokens,
 *        skipping any token for which @p Predicate returns false.
 *
 * Constructed via `view | Filter(pred)` or, for the common "skip empty
 * tokens" case, `view | SkipEmpty`.
 *
 * @tparam UnderlyingView Range type yielding `std::string_view` tokens.
 * @tparam Predicate      Callable with signature
 *                       `bool(std::string_view)`.
 */
template <typename UnderlyingView, typename Predicate>
struct FilterView {
  /// Iterator that skips tokens failing the predicate.
  struct Iterator
      : public ViewIteratorBase<Iterator, typename UnderlyingView::Iterator> {
    /// End of the underlying range, used to bound the search for a valid
    /// token.
    typename UnderlyingView::Iterator base_end_;
    /// Predicate determining which tokens are kept.
    Predicate pred_;

    /// @return The current (already predicate-satisfying) token.
    std::string_view operator*() const { return *(this->base_it_); }

    /// Advances to the next token satisfying the predicate, or to the
    /// end. @return Reference to `*this`.
    Iterator& operator++() {
      ++(this->base_it_);
      AdvanceToValid();
      return *this;
    }

    /// @return Whether both iterators wrap equal underlying positions.
    Bool operator==(const Iterator& other) const {
      return this->base_it_ == other.base_it_;
    }

    /// Advances `base_it_` until it points at a token satisfying
    /// #pred_, or until it reaches #base_end_.
    void AdvanceToValid() {
      while (this->base_it_ != base_end_ && !pred_(*(this->base_it_))) {
        ++(this->base_it_);
      }
    }
  };

  /**
   * @brief Constructs a filtering adapter over @p view.
   * @param view Underlying view of tokens to filter.
   * @param pred Predicate selecting which tokens to keep.
   */
  FilterView(UnderlyingView view, Predicate pred)
      : view_(axio::Move(view)), pred_(pred) {}

  /// @return An iterator at the first token satisfying the predicate.
  Iterator begin() {
    auto it = Iterator{{view_.begin()}, {view_.end()}, pred_};
    it.AdvanceToValid();
    return it;
  }

  /// @return The past-the-end iterator.
  Iterator end() { return Iterator{{view_.end()}, {view_.end()}, pred_}; }

 private:
  /// Underlying view being adapted.
  UnderlyingView view_;
  /// Predicate selecting which tokens to keep.
  Predicate pred_;
};

/// Tag type, parameterized by a predicate, used with `operator|` to build
/// a FilterView. See #Filter.
template <typename Predicate>
struct FilterAdapter {
  /// Predicate selecting which tokens to keep.
  Predicate pred;
};

/**
 * @brief Creates a pipeable adapter that filters a token view using
 *        @p pred, e.g. `view | axio::Filter([](auto s){ return ...; })`.
 * @tparam Predicate Callable with signature `bool(std::string_view)`.
 * @param pred Predicate selecting which tokens to keep.
 * @return A FilterAdapter that can be piped onto a token view.
 */
template <typename Predicate>
inline auto Filter(Predicate&& pred) {
  using DecayedPred = Decay_T<Predicate>;
  return FilterAdapter<DecayedPred>{axio::Forward<Predicate>(pred)};
}

/**
 * @brief Pipe operator that wraps @p view in a FilterView using
 *        @p adaptor's predicate.
 * @tparam UnderlyingView Range type yielding `std::string_view` tokens.
 * @tparam Predicate      Predicate type held by @p adaptor.
 * @param view    Underlying view to adapt.
 * @param adaptor Adapter created by Filter().
 * @return A FilterView over @p view.
 */
template <typename UnderlyingView, typename Predicate>
inline auto operator|(UnderlyingView view, FilterAdapter<Predicate> adaptor) {
  return FilterView<UnderlyingView, Predicate>(axio::Move(view),
                                               axio::Move(adaptor.pred));
}

/// Tag type used with `operator|` to build a FilterView that drops empty
/// tokens. See #SkipEmpty.
struct SkipEmptyAdapter {};
/// Pipe this onto a view of tokens to lazily drop empty ones, e.g.
/// `axio::Split(s, ',') | axio::SkipEmpty`.
inline constexpr SkipEmptyAdapter SkipEmpty;

/**
 * @brief Pipe operator that wraps @p view in a FilterView dropping empty
 *        tokens.
 * @tparam UnderlyingView Range type yielding `std::string_view` tokens.
 * @param view Underlying view to adapt.
 * @return A FilterView over @p view that skips empty tokens.
 */
template <typename UnderlyingView>
inline auto operator|(UnderlyingView view, SkipEmptyAdapter) {
  return FilterView(axio::Move(view),
                    [](std::string_view s) noexcept { return !s.empty(); });
}

/**
 * @brief Lazily adapts an underlying view of `std::string_view` tokens,
 *        limiting iteration to at most the first @p n tokens.
 *
 * Constructed via `view | Take(n)`.
 *
 * @tparam UnderlyingView Range type yielding `std::string_view` tokens.
 */
template <typename UnderlyingView>
struct TakeView {
  /// Iterator type of the underlying view.
  using UnderlyingIterator = decltype(std::declval<UnderlyingView>().begin());

  /// Iterator that additionally tracks a remaining-element countdown.
  struct Iterator
      : public ViewIteratorBase<Iterator, typename UnderlyingView::Iterator> {
    using Base = ViewIteratorBase<Iterator, typename UnderlyingView::Iterator>;

    /**
     * @brief Constructs an iterator at @p base_it with @p n elements
     *        remaining to be yielded.
     * @param base_it Underlying position.
     * @param n       Number of elements still to yield from this
     *                position.
     */
    Iterator(typename UnderlyingView::Iterator base_it, SizeT n)
        : Base{base_it}, count_(n) {}

    /// @return The current underlying token.
    std::string_view operator*() const { return *(this->base_it_); }

    /// Advances the underlying iterator and decrements the remaining
    /// count (if not already zero). @return Reference to `*this`.
    Iterator& operator++() {
      ++this->base_it_;
      if (count_ > 0) {
        --count_;
      }
      return *this;
    }

    /// @return True if both iterators have the same remaining count
    ///         (covers reaching the `Take` limit), or if their
    ///         underlying positions are equal (covers reaching the end
    ///         of a shorter underlying view).
    Bool operator==(const Iterator& other) const {
      if (count_ == other.count_) {
        return true;
      }
      return this->base_it_ == other.base_it_;
    }

   private:
    /// Number of elements still to be yielded; reaching zero signals
    /// end-of-range for `Take`'s purposes.
    SizeT count_;
  };

  /**
   * @brief Constructs a limiting adapter over @p view.
   * @param view Underlying view of tokens.
   * @param n    Maximum number of tokens to yield.
   */
  TakeView(UnderlyingView view, SizeT n) : view_(axio::Move(view)), n_(n) {}
  /// @return An iterator at the first token, with #n_ remaining.
  auto begin() { return Iterator(view_.begin(), n_); }
  /// @return The past-the-end iterator (zero remaining).
  auto end() { return Iterator(view_.end(), 0); }

 private:
  /// Underlying view being adapted.
  UnderlyingView view_;
  /// Maximum number of tokens to yield.
  SizeT n_;
};

/// Tag type, parameterized by a count, used with `operator|` to build a
/// TakeView. See #Take.
struct TakeAdapter {
  SizeT n;
};

/**
 * @brief Creates a pipeable adapter limiting a token view to its first
 *        @p n tokens, e.g. `view | axio::Take(3)`.
 * @param n Maximum number of tokens to yield.
 * @return A TakeAdapter that can be piped onto a token view.
 */
inline auto Take(SizeT n) {
  return TakeAdapter{n};
}

/**
 * @brief Pipe operator that wraps @p view in a TakeView limited to
 *        @p adapter's count.
 * @tparam UnderlyingView Range type yielding `std::string_view` tokens.
 * @param view    Underlying view to adapt.
 * @param adapter Adapter created by Take().
 * @return A TakeView over @p view.
 */
template <typename UnderlyingView>
inline auto operator|(UnderlyingView&& view, TakeAdapter adapter) {
  return TakeView<Decay_T<UnderlyingView>>(axio::Forward<UnderlyingView>(view),
                                           adapter.n);
}

/**
 * @brief Lazily adapts an underlying view of `std::string_view` tokens,
 *        skipping the first @p n tokens.
 *
 * Constructed via `view | Drop(n)`. Unlike TakeView, the skip is applied
 * eagerly when `begin()` is called (advancing up to @p n times), rather
 * than per-iteration.
 *
 * @tparam UnderlyingView Range type yielding `std::string_view` tokens.
 */
template <typename UnderlyingView>
struct DropView {
  /// Iterator type of the underlying view.
  using BaseIteratorType = decltype(std::declval<UnderlyingView>().begin());

  /// Thin pass-through iterator over the post-skip underlying range.
  struct Iterator : public ViewIteratorBase<Iterator, BaseIteratorType> {
    using Base = ViewIteratorBase<Iterator, BaseIteratorType>;

    /**
     * @brief Constructs an iterator wrapping the (already advanced)
     *        underlying position @p base_it.
     * @param base_it Underlying position.
     */
    Iterator(BaseIteratorType base_it) : Base{base_it} {}

    /// @return The current underlying token.
    std::string_view operator*() const { return *(this->base_it_); }

    /// Advances the underlying iterator. @return Reference to `*this`.
    Iterator& operator++() {
      ++this->base_it_;
      return *this;
    }

    /// @return Whether both iterators wrap equal underlying positions.
    bool operator==(const Iterator& other) const {
      return this->base_it_ == other.base_it_;
    }
  };

  /**
   * @brief Constructs a skipping adapter over @p view.
   * @param view Underlying view of tokens.
   * @param n    Number of leading tokens to skip.
   */
  DropView(UnderlyingView view, SizeT n) : view_(axio::Move(view)), n_(n) {}

  /// @return An iterator positioned after skipping up to #n_ leading
  ///         tokens (fewer, if the underlying view is shorter).
  auto begin() {
    auto it = view_.begin();
    auto end_it = view_.end();
    for (SizeT i = 0; i < n_ && it != end_it; ++i) {
      ++it;
    }

    return Iterator(it);
  }

  /// @return The past-the-end iterator.
  auto end() { return Iterator(view_.end()); }

 private:
  /// Underlying view being adapted.
  UnderlyingView view_;
  /// Number of leading tokens to skip.
  SizeT n_;
};

/// Tag type, parameterized by a count, used with `operator|` to build a
/// DropView. See #Drop.
struct DropAdapter {
  /// Number of leading tokens to skip.
  SizeT n;
};

/**
 * @brief Creates a pipeable adapter skipping the first @p n tokens of a
 *        view, e.g. `view | axio::Drop(1)`.
 * @param n Number of leading tokens to skip.
 * @return A DropAdapter that can be piped onto a token view.
 */
inline auto Drop(SizeT n) {
  return DropAdapter{n};
}

/**
 * @brief Pipe operator that wraps @p view in a DropView skipping
 *        @p adapter's count of leading tokens.
 * @tparam UnderlyingView Range type yielding `std::string_view` tokens.
 * @param view    Underlying view to adapt.
 * @param adapter Adapter created by Drop().
 * @return A DropView over @p view.
 */
template <typename UnderlyingView>
inline auto operator|(UnderlyingView&& view, DropAdapter adapter) {
  return DropView<Decay_T<UnderlyingView>>(axio::Forward<UnderlyingView>(view),
                                           adapter.n);
}

namespace detail {
/**
 * @brief Resolves the element type to construct when materializing a
 *        view into a container: prefers the container's `ValueType`
 *        (axio convention) and falls back to `value_type` (standard
 *        convention).
 * @tparam T Container type.
 */
template <typename T, typename = void>
struct ExtractValueType {
  using type = typename T::value_type;
};

/// Specialization selected when @p T defines `ValueType` (axio
/// containers).
template <typename T>
struct ExtractValueType<T, Void_T<typename T::ValueType>> {
  using type = typename T::ValueType;
};

/// Detection expression: well-formed if `T` has a `push_back(V)` member
/// (standard-library container convention).
template <typename T, typename V>
using push_back_op = decltype(std::declval<T>().push_back(std::declval<V>()));

/// Detection expression: well-formed if `T` has a `Push(V)` member (axio
/// container convention).
template <typename T, typename V>
using Push_Op = decltype(std::declval<T>().Push(std::declval<V>()));
}  // namespace detail

/// Tag type, parameterized by the target container, used with
/// `operator|` to materialize a token view. See #To.
template <typename Container>
struct ToContainerAdaptor {};

/**
 * @brief Creates a pipeable adapter that materializes a token view into
 *        a @p Container, e.g.
 *        `axio::Split(s, ',') | axio::To<std::vector<axio::String>>()`.
 * @tparam Container Target container type.
 * @return A ToContainerAdaptor that can be piped onto a token view.
 */
template <typename Container>
inline auto To() {
  return ToContainerAdaptor<Container>{};
}

/**
 * @brief Pipe operator that consumes @p view and constructs a
 *        @p Container from its tokens.
 *
 * Each token is converted to the container's element type and inserted
 * using, in order of preference: `push_back` (standard containers),
 * `Push` (axio containers), or a generic `std::inserter` otherwise (e.g.
 * for associative containers).
 *
 * @tparam UnderlyingView Range type yielding `std::string_view` tokens.
 * @tparam Container      Target container type.
 * @param view Underlying view whose tokens are inserted into the result.
 * @return A newly constructed @p Container holding the converted tokens.
 */
template <typename UnderlyingView, typename Container>
inline auto operator|(UnderlyingView&& view, ToContainerAdaptor<Container>) {
  using ValueType = typename detail::ExtractValueType<Container>::type;

  Container container;

  for (auto token : view) {
    if constexpr (IsDetected_V<detail::push_back_op, Container, ValueType>) {
      container.push_back(ValueType(token));
    } else if constexpr (IsDetected_V<detail::Push_Op, Container, ValueType>) {
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