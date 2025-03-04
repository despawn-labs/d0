#pragma once

#include "d0/defs.h"

#include <type_traits>

namespace d0 {

template <typename T, typename E> class Result;

template <typename T> class Ok {
public:
  explicit constexpr Ok(const T &value) : value_(value) {}
  explicit constexpr Ok(T &&value) : value_(std::move(value)) {}

  constexpr const T &Value() const & { return value_; }
  constexpr T &&Value() && { return std::move(value_); }

private:
  T value_;
};

template <typename E> class Err {};

namespace detail {

enum class State {
  kInvalid = -1,
  kOk = 0,
  kError = 1,
};

template <typename T, typename E, typename C = void, typename D = void>
class ResultStorage {
public:
  constexpr explicit ResultStorage(Ok<T> o) : state_{State::kOk} {
    inner_.t = std::move(o).Value();
  }
  constexpr explicit ResultStorage(Err<E> e) : state_{State::kError} {
    inner_.e = std::move(e).Value();
  }

  ~ResultStorage() {
    switch (state_) {
    case State::kOk:
      ~inner_.T();
      break;
    case State::kError:
      ~inner_.E();
      break;
    default:
      break;
    }
  }

  [[nodiscard]] constexpr const State &GetState() const & { return state_; }

private:
  State state_;
  union {
    T t;
    E e;
    u8 i;
  } inner_;
};

template <typename T, typename E>
class ResultStorage<T, E, std::enable_if_t<std::is_trivially_destructible_v<T>>,
                    std::enable_if_t<std::is_trivially_destructible_v<E>>> {};

} // namespace detail

template <typename T, typename E> class Result {
public:
  constexpr explicit Result(Ok<T> o) : storage_(std::move(o)) {}
  constexpr explicit Result(Err<E> e) : storage_(std::move(e)) {}

  [[nodiscard]] constexpr bool IsOk() const {
    return storage_.State() == detail::State::kOk;
  }
  [[nodiscard]] constexpr bool IsErr() const {
    return storage_.State() == detail::State::kError;
  }

  ~Result() = default;

private:
  detail::ResultStorage<T, E> storage_;
};

} // namespace d0