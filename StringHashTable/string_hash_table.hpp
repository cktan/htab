/// \file
/// \brief String hash table

#include "utils.hpp"
#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <unordered_map>
#include <smmintrin.h>

//TODO:remove ALWAYS_INLINE

namespace detail {

struct string_key0 {};

using string_key8 = uint64_t;

struct string_key16 {
  uint64_t a;
  uint64_t b;
};

struct string_key24 {
  uint64_t a;
  uint64_t b;
  uint64_t c;
};

struct string_key_str {  // for string keys with length > 24 chars
  // Note: by design, string hash table and its keys may exist independently. So, we share
  // string data (as raw char buffer, concretely).
  std::shared_ptr<char[]> data;
  size_t size;
  size_t hash;
};

inline bool ALWAYS_INLINE operator==(string_key0, string_key0) { return true; }
inline bool ALWAYS_INLINE operator==(string_key16 left, string_key16 right) {
  return left.a == right.a && left.b == right.b;
}
inline bool ALWAYS_INLINE operator==(const string_key24 &left, const string_key24 &right) {
  return left.a == right.a && left.b == right.b && left.c == right.c;
}
inline bool ALWAYS_INLINE operator==(const string_key_str &left, const string_key_str &right) {
  return left.data == right.data
    || (left.size == right.size && !memcmp(left.data.get(), right.data.get(), left.size));
}

enum key_type {
  key_type0,
  key_type8,
  key_type16,
  key_type24,
  key_type_str
};

inline key_type ALWAYS_INLINE map_size_to_key_type(size_t size) {
  if (size > 24) return key_type_str;
  if (!size) return key_type0;
  --size >>= 3;
  if (!size) return key_type8;
  size >>= 1;
  if (!size) return key_type16;
  return key_type24;
}

inline size_t ALWAYS_INLINE hash(std::string_view sv) {
  size_t res = size_t(-1ULL);
  size_t sz = std::size(sv);
  const char *p = std::data(sv);
  const char *lp = p + sz - 8; // starting pointer of the last 8 bytes segment
  char s = (-sz & 7) * 8; // pending bits that needs to be shifted out
  uint64_t n[3]; // std::string_view in SSO map will have length > 24
  memcpy(&n, p, 24);
  res = _mm_crc32_u64(res, n[0]);
  res = _mm_crc32_u64(res, n[1]);
  res = _mm_crc32_u64(res, n[2]);
  p += 24;
  while (p + 8 < lp) {
    memcpy(&n[0], p, 8);
    res = _mm_crc32_u64(res, n[0]);
    p += 8;
  }
  memcpy(&n[0], lp, 8);
  n[0] >>= s;
  res = _mm_crc32_u64(res, n[0]);
  return res;
}

inline int ALWAYS_INLINE shifting_bits(std::string_view sv) { return (-std::size(sv) & 7) << 3; };

inline string_key8 ALWAYS_INLINE to_string_key8(std::string_view sv) {
  string_key8 ret;
  if ((reinterpret_cast<uintptr_t>(std::data(sv)) & 2048) == 0) { // first half page
    memcpy(&ret, std::data(sv), 8);
    ret &= uint64_t(-1) >> shifting_bits(sv);
  } else {
    memcpy(&ret, std::data(sv) + std::size(sv) - 8, 8);
    ret >>= shifting_bits(sv);
  }
  return ret;
}
inline string_key16 ALWAYS_INLINE to_string_key16(std::string_view sv) {
  string_key16 ret;
  memcpy(&ret.a, std::data(sv), 8);
  memcpy(&ret.b, std::data(sv) + std::size(sv) - 8, 8);
  ret.b >>= shifting_bits(sv);
  return ret;
}
inline string_key24 ALWAYS_INLINE to_string_key24(std::string_view sv) {
  string_key24 ret;
  memcpy(&ret.a, std::data(sv), 16);
  memcpy(&ret.c, std::data(sv) + std::size(sv) - 8, 8);
  ret.c >>= shifting_bits(sv);
  return ret;
}
inline string_key_str ALWAYS_INLINE to_string_key_str(std::string_view sv) {
  char *data = new char[std::size(sv)];
  memcpy(data, std::data(sv), std::size(sv));
  return string_key_str{std::shared_ptr<char[]>(data), std::size(sv), hash(sv)};
}

// Warning: passing input parameter by ref. is mandatory - otherwise string_view will point to
// stack memory! It was a subtle bug.
inline std::string_view ALWAYS_INLINE to_string_view(const string_key0 &) { return {}; }
inline std::string_view ALWAYS_INLINE to_string_view(const string_key8 &key) {
  return {reinterpret_cast<const char *>(&key), 8ul - (__builtin_clzll(key) >> 3)};
}
inline std::string_view ALWAYS_INLINE to_string_view(const string_key16 &key) {
  return {reinterpret_cast<const char *>(&key), 16ul - (__builtin_clzll(key.b) >> 3)};
}
inline std::string_view ALWAYS_INLINE to_string_view(const string_key24 &key) {
  return {reinterpret_cast<const char *>(&key), 24ul - (__builtin_clzll(key.c) >> 3)};
}
inline std::string_view ALWAYS_INLINE to_string_view(const string_key_str &key) {
  return std::string_view(key.data.get(), key.size);
}

struct hasher_t {
  size_t ALWAYS_INLINE operator()(string_key0) const { return 0; }
  size_t ALWAYS_INLINE operator()(string_key8 key) const {
    size_t res = size_t(-1ULL);
    res = _mm_crc32_u64(res, key);
    return res;
  }
  size_t ALWAYS_INLINE operator()(string_key16 key) const {
    size_t res = size_t(-1ULL);
    res = _mm_crc32_u64(res, key.a);
    res = _mm_crc32_u64(res, key.b);
    return res;
  }
  size_t ALWAYS_INLINE operator()(const string_key24 &key) const {
    size_t res = size_t(-1ULL);
    res = _mm_crc32_u64(res, key.a);
    res = _mm_crc32_u64(res, key.b);
    res = _mm_crc32_u64(res, key.c);
    return res;
  }
  size_t ALWAYS_INLINE operator()(const string_key_str &key) const { return key.hash; }
};

} // detail::

// string_hash_key_t

/// User side key to be used with string_hash_table_t.
/// string_hash_key_t proxies std::string_view as user key, but in addition it copies pointed
/// string (so, the last can be freed) and stores it in most appropriate format for fast processing.
/// Long strings (> 24 chars) are stored along with their precalculated hashes.
class string_hash_key_t {
public:
  string_hash_key_t() {}
  string_hash_key_t(std::string_view sv) : m_data(to_data(sv)) {}
  string_hash_key_t(const char *s) : string_hash_key_t(std::string_view(s)) {}
  string_hash_key_t(const std::string &s) : string_hash_key_t(std::string_view(s)) {}

  std::string_view to_string_view() const {
    auto callback = [](const auto &obj) { return detail::to_string_view(obj); };
    return std::visit(callback, m_data);
  }
  operator std::string_view() const { return to_string_view(); }

private:
  using data_t = std::variant<detail::string_key0, detail::string_key8, detail::string_key16,
    detail::string_key24, detail::string_key_str>;  // must follow detail::key_type enum
  data_t m_data;

  static data_t to_data(std::string_view sv) {
    switch (detail::map_size_to_key_type(std::size(sv))) {
      case detail::key_type0: return detail::string_key0();
      case detail::key_type8: return detail::to_string_key8(sv);
      case detail::key_type16: return detail::to_string_key16(sv);
      case detail::key_type24: return detail::to_string_key24(sv);
      case detail::key_type_str: return detail::to_string_key_str(sv);
      default: UNREACHABLE();
    };
  }

  template <typename T> friend class string_hash_table_t;
  // Used by string_hash_table_t:
  template <typename T>
  string_hash_key_t(const T &string_key) : m_data(string_key) {}
  detail::key_type type() const { return detail::key_type(m_data.index()); }
  template <size_t I>
  auto get_string_key() const { return std::get<I>(m_data); }
};

// string_hash_table_t

template <typename T>
class string_hash_table_t {
public:
  using key_type = string_hash_key_t;
  using mapped_type = T;

  string_hash_table_t() {}
  string_hash_table_t(size_t elem_count) { reserve(elem_count); }  // elements, not buckets!

  void reserve(size_t elem_count) {
    if (elem_count < 5) {
      elem_count = 5;
    }
    size_t subcount = --elem_count / 4; // 1 element for m0
    m1.reserve(subcount);
    m2.reserve(subcount);
    m3.reserve(subcount);
    ms.reserve(elem_count - subcount * 3);
  }

  bool empty() const noexcept {
    return m0.empty() && m1.empty() && m2.empty() && m3.empty() && ms.empty();
  }

  size_t size() const noexcept {
    return m0.size() + m1.size() + m2.size() + m3.size() + ms.size();
  }

  void clear() noexcept {
    m0.clear();
    m1.clear();
    m2.clear();
    m3.clear();
    ms.clear();
  }

  mapped_type *find(const key_type &key);
  template <typename... Args>
  std::pair<mapped_type *, bool> try_emplace(const key_type &key, Args &&... args);
  bool erase(const key_type &key);

  template<typename F>
  void for_each(F &&f) {
    for (const auto &[first, second] : m0) {
      f(first, second);
    }
    for (const auto &[first, second] : m1) {
      f(first, second);
    }
    for (const auto &[first, second] : m2) {
      f(first, second);
    }
    for (const auto &[first, second] : m3) {
      f(first, second);
    }
    for (const auto &[first, second] : ms) {
      f(first, second);
    }
  }

private:
  //OPTIMIZATION: using a custom fake container (that mimics std::unordered_map)
  // to store a single value for string_key0, we save a bucket size bytes of
  // memory.
  std::unordered_map<detail::string_key0, T, detail::hasher_t> m0;
  std::unordered_map<detail::string_key8, T, detail::hasher_t> m1;
  std::unordered_map<detail::string_key16, T, detail::hasher_t> m2;
  std::unordered_map<detail::string_key24, T, detail::hasher_t> m3;
  std::unordered_map<detail::string_key_str, T, detail::hasher_t> ms;

  template <typename Func>
  inline decltype(auto) ALWAYS_INLINE dispatch(const key_type &key, Func &&func);
  template <typename... Args>
  std::pair<mapped_type *, bool> emplace(const key_type &key, Args &&... args);
};

template <typename T>
template <typename Func>
decltype(auto) string_hash_table_t<T>::dispatch(const key_type &key, Func &&func) {
  switch (key.type()) {
    case detail::key_type0: return func(m0, key.get_string_key<detail::key_type0>());
    case detail::key_type8: return func(m1, key.get_string_key<detail::key_type8>());
    case detail::key_type16: return func(m2, key.get_string_key<detail::key_type16>());
    case detail::key_type24: return func(m3, key.get_string_key<detail::key_type24>());
    case detail::key_type_str: return func(ms, key.get_string_key<detail::key_type_str>());
    default: UNREACHABLE();
  };
}

template <typename T>
typename string_hash_table_t<T>::mapped_type *string_hash_table_t<T>::find(const key_type &key) {
  auto callback = [](auto &map, auto key) -> mapped_type * {
    auto it = map.find(key);
    return map.end() != it ? &it->second : nullptr;
  };
  return dispatch(key, callback);
}

template <typename T>
template <typename... Args>
std::pair<typename string_hash_table_t<T>::mapped_type *, bool> string_hash_table_t<T>::emplace(
  const key_type &key, Args &&... args) {
  // Note: There is failed to determine rvalue ref. after forwarding as tuple, i.e. when
  // input args start from rvalue (obtained from std::move()), the following returns false:
  // auto m_args = std::forward_as_tuple(args...);
  // (std::is_rvalue_reference_v<std::tuple_element_t<0, decltype(m_args)>>) -> returns false,
  // but the following is ok:
  // (std::is_rvalue_reference_v<std::tuple_element_t<0, std::tuple<Args &&...>>>) -> returns true.

  // Note: We cannot use input parameter pack inside a lambda directly, we need to capture it
  // somehow or pass as parameter. In C++20 a lambda can capture parameter pack:
  // [... args = std::forward<Args>(args)]() { use args }
  // In C++17 we need to use tuple.

  using t0 = std::tuple_element_t<0, std::tuple<Args &&...>>;
  auto targs = std::forward_as_tuple(args...);

  if constexpr (sizeof...(Args) == 1 && std::is_rvalue_reference_v<t0>
    && std::is_same_v<mapped_type, std::decay_t<t0>>) {
    // scalar 'mapped_type &&'
    auto callback = [&targs](auto &map, auto key) -> std::pair<mapped_type *, bool> {
      auto [it, inserted] = map.emplace(key, std::forward<mapped_type>(std::get<0>(targs)));
      return {&it->second, inserted};
    };
    return dispatch(key, callback);
  } else {
    // tuple of input args ('mapped_type &' is here)
    auto callback = [&targs](auto &map, auto key) -> std::pair<mapped_type *, bool> {
      auto [it, inserted] = map.emplace(std::piecewise_construct,
                                        std::forward_as_tuple(key), targs);
      return {&it->second, inserted};
    };
    return dispatch(key, callback);
  }
}

template <typename T>
template <typename... Args>
std::pair<typename string_hash_table_t<T>::mapped_type *, bool> string_hash_table_t<T>::try_emplace(
  const key_type &key, Args &&... args) {
  // Note: Alternatively, just use std::unordered_map::try_emplace().
  mapped_type *value = find(key);
  if (value) return {value, false};
  return emplace(key, std::forward<Args>(args)...);
}

template <typename T>
bool string_hash_table_t<T>::erase(const key_type &key) {
  auto callback = [](auto &map, auto key) -> bool {
    return (map.erase(key));
  };
  return dispatch(key, callback);
}


/* ==TRASH==
*/
