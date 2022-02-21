/// \file
/// \brief String hash table

#include <cstring>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <smmintrin.h>

#ifdef DEBUG
  #define ALWAYS_INLINE
#else
  #define ALWAYS_INLINE __attribute__((__always_inline__))
#endif

#define CASE_1_8 \
  case 1: \
  case 2: \
  case 3: \
  case 4: \
  case 5: \
  case 6: \
  case 7: \
  case 8

#define CASE_9_16 \
  case 9: \
  case 10: \
  case 11: \
  case 12: \
  case 13: \
  case 14: \
  case 15: \
  case 16

#define CASE_17_24 \
  case 17: \
  case 18: \
  case 19: \
  case 20: \
  case 21: \
  case 22: \
  case 23: \
  case 24

namespace detail {

struct string_key0 {};
static bool operator==(string_key0, string_key0) { return true; }

using string_key8 = uint64_t;

struct string_key16 {
  uint64_t a;
  uint64_t b;

  bool operator==(const string_key16 rhs) const { return a == rhs.a && b == rhs.b; }
  bool operator!=(const string_key16 rhs) const { return !operator==(rhs); }
  bool operator==(const uint64_t rhs) const { return a == rhs && b == 0; }
  bool operator!=(const uint64_t rhs) const { return !operator==(rhs); }

  string_key16 & operator=(const uint64_t rhs) {
    a = rhs;
    b = 0;
    return *this;
  }
};

struct string_key24 {
  uint64_t a;
  uint64_t b;
  uint64_t c;

  bool operator==(const string_key24 rhs) const { return a == rhs.a && b == rhs.b && c == rhs.c; }
  bool operator!=(const string_key24 rhs) const { return !operator==(rhs); }
  bool operator==(const uint64_t rhs) const { return a == rhs && b == 0 && c == 0; }
  bool operator!=(const uint64_t rhs) const { return !operator==(rhs); }

  string_key24 & operator=(const uint64_t rhs) {
    a = rhs;
    b = 0;
    c = 0;
    return *this;
  }
};

inline std::string_view ALWAYS_INLINE to_string_view(const string_key0 &) { return {}; }

inline std::string_view ALWAYS_INLINE to_string_view(const string_key8 &n) {
  return {reinterpret_cast<const char *>(&n), 8ul - (__builtin_clzll(n) >> 3)};
}

inline std::string_view ALWAYS_INLINE to_string_view(const string_key16 &n) {
  return {reinterpret_cast<const char *>(&n), 16ul - (__builtin_clzll(n.b) >> 3)};
}

inline std::string_view ALWAYS_INLINE to_string_view(const string_key24 &n) {
  return {reinterpret_cast<const char *>(&n), 24ul - (__builtin_clzll(n.c) >> 3)};
}

inline const std::string_view & ALWAYS_INLINE to_string_view(const std::string_view &s) {
  return s;
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

  size_t ALWAYS_INLINE operator()(string_key24 key) const {
    size_t res = size_t(-1ULL);
    res = _mm_crc32_u64(res, key.a);
    res = _mm_crc32_u64(res, key.b);
    res = _mm_crc32_u64(res, key.c);
    return res;
  }

  size_t ALWAYS_INLINE operator()(std::string_view key) const {
    size_t res = size_t(-1ULL);
    size_t sz = std::size(key);
    const char *p = std::data(key);
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
};

} // detail::

template <typename T>
class string_hash_table_t {
public:
  // This class isn't a STL compatible container, because internally it uses
  // containers of different types to maintain varied lenght strings. So, there
  // are no unified types or iterator exist, and in public interface we don't use
  // such types or iterators at all.

  using key_type = std::string_view;
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

  inline mapped_type * ALWAYS_INLINE find(key_type key);
  inline std::pair<mapped_type *, bool> ALWAYS_INLINE insert(key_type key,
                                                             const mapped_type &value);
  inline std::pair<mapped_type *, bool> ALWAYS_INLINE insert(key_type key,
                                                             mapped_type &&value);
  template <typename... Args>
  inline std::pair<mapped_type *, bool> ALWAYS_INLINE emplace(key_type key, Args &&... args);
  template <typename... Args>
  inline std::pair<mapped_type *, bool> ALWAYS_INLINE try_emplace(key_type key, Args &&... args);
  inline bool ALWAYS_INLINE erase(key_type key);

  template<typename F>
  void for_each(F f) {
    for (const auto &[first, second] : m0) {
      f(detail::to_string_view(first), second);
    }
    for (const auto &[first, second] : m1) {
      f(detail::to_string_view(first), second);
    }
    for (const auto &[first, second] : m2) {
      f(detail::to_string_view(first), second);
    }
    for (const auto &[first, second] : m3) {
      f(detail::to_string_view(first), second);
    }
    for (const auto &[first, second] : ms) {
      f(detail::to_string_view(first), second);
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
  std::unordered_map<std::string_view, T, detail::hasher_t> ms;

  template <typename Func>
  inline decltype(auto) ALWAYS_INLINE dispatch(key_type x, Func func);

  struct find_callable {
    template <typename Map>
    mapped_type * ALWAYS_INLINE operator()(Map &map, typename Map::key_type key) {
      auto it = map.find(key);
      return map.end() != it ? &it->second : nullptr;
    }
  };

  template <typename V>
  struct insert_callable {
    V m_value;  // either being 'const V &' or 'V &&'
    insert_callable(V value) : m_value(std::forward<V>(value)) {}
    template <typename Map>
    std::pair<mapped_type *, bool> ALWAYS_INLINE operator()(Map &map, typename Map::key_type key) {
      // Note: insert() way makes copy of mapped_type while constructing pair (value_type):
      // auto [it, inserted] = map.insert({key, std::forward<V>(m_value)});
      auto [it, inserted] = map.emplace(key, std::forward<V>(m_value));
      return {&it->second, inserted};
    }
  };

  template <typename U>
  struct emplace_callable {
    U m_args;  // either being scalar 'U &&' or tuple of input args ('U &' is here)
    emplace_callable(U args) : m_args(std::forward<U>(args)) {}
    template <typename Map>
    std::pair<mapped_type *, bool> ALWAYS_INLINE operator()(Map &map, typename Map::key_type key) {
      if constexpr (std::is_same_v<mapped_type, std::decay_t<U>>) {  // scalar 'U &&'
        auto [it, inserted] = map.emplace(key, std::forward<mapped_type>(m_args));
        return {&it->second, inserted};
      } else {  // tuple of input args ('U &' is here)
        auto [it, inserted] = map.emplace(std::piecewise_construct, std::forward_as_tuple(key),
                                          m_args);
        return {&it->second, inserted};
      }
    }
  };

  template <typename U>
  struct try_emplace_callable {
    U m_args;  // either being scalar 'U &&' or tuple of input args ('U &' is here)
    try_emplace_callable(U args) : m_args(std::forward<U>(args)) {}
    template <typename Map>
    std::pair<mapped_type *, bool> ALWAYS_INLINE operator()(Map &map, typename Map::key_type key) {
      if constexpr (std::is_same_v<mapped_type, std::decay_t<U>>) {  // scalar 'U &&'
        auto [it, inserted] = map.try_emplace(key, std::forward<mapped_type>(m_args));
        return {&it->second, inserted};
      } else {  // tuple of input args ('U &' is here)
        auto func = [&](auto &&... args) -> auto {
          return map.try_emplace(key, std::forward<decltype(args)>(args)...);
        };
        auto [it, inserted] = std::apply(func, m_args);
        return {&it->second, inserted};
      }
    }
  };

  struct erase_callable {
    template <typename Map>
    bool ALWAYS_INLINE operator()(Map &map, typename Map::key_type key) {
      return (map.erase(key));
    }
  };
};

template <typename T>
template <typename Func>
decltype(auto) string_hash_table_t<T>::dispatch(key_type key, Func func) {
  // Dispatch is written in a way that maximizes the performance:
  // 1. Always memcpy 8 times bytes
  // 2. Use switch case extension to generate fast dispatching table
  // 3. [DELETED] Combine hash computation along with key loading
  // 4. Funcs are named callables that can be force_inlined
  // NOTE: It relies on Little Endianness and SSE4.2
  static constexpr detail::string_key0 key0;
  size_t sz = std::size(key);
  const char *p = std::data(key);
  char s = (-sz & 7) * 8; // pending bits that needs to be shifted out
  union {
    detail::string_key8 k8;
    detail::string_key16 k16;
    detail::string_key24 k24;
    uint64_t n[3];
  };
  switch (sz) {
    case 0: {
      return func(m0, key0);
    }
    CASE_1_8 : {
      if ((reinterpret_cast<uintptr_t>(p) & 2048) == 0) { // first half page
        memcpy(&n[0], p, 8);
        n[0] &= -1ul >> s;
      }
      else {
        const char *lp = std::data(key) + std::size(key) - 8;
        memcpy(&n[0], lp, 8);
        n[0] >>= s;
      }
      return func(m1, k8);
    }
    CASE_9_16 : {
      memcpy(&n[0], p, 8);
      const char *lp = std::data(key) + std::size(key) - 8;
      memcpy(&n[1], lp, 8);
      n[1] >>= s;
      return func(m2, k16);
    }
    CASE_17_24 : {
      memcpy(&n[0], p, 16);
      const char *lp = std::data(key) + std::size(key) - 8;
      memcpy(&n[2], lp, 8);
      n[2] >>= s;
      return func(m3, k24);
    }
    default: {
      return func(ms, key);
    }
  }
}

template <typename T>
typename string_hash_table_t<T>::mapped_type *string_hash_table_t<T>::find(key_type key) {
  return dispatch(key, find_callable());
}

template <typename T>
std::pair<typename string_hash_table_t<T>::mapped_type *, bool> string_hash_table_t<T>::insert(
  key_type key, const mapped_type &value) {
  return dispatch(key, insert_callable<const T &>(value));
}

template <typename T>
std::pair<typename string_hash_table_t<T>::mapped_type *, bool> string_hash_table_t<T>::insert(
  key_type key, mapped_type &&value) {
  return dispatch(key, insert_callable<T &&>(std::forward<mapped_type>(value)));
}

template <typename T>
template <typename... Args>
std::pair<typename string_hash_table_t<T>::mapped_type *, bool> string_hash_table_t<T>::emplace(
  key_type key, Args &&... args) {
  // Note: There is failed to determine rvalue ref. after forwarding as tuple, i.e. when
  // input args start from rvalue (obtained from std::move()), the following returns false:
  // auto m_args = std::forward_as_tuple(args...);
  // (std::is_rvalue_reference_v<std::tuple_element_t<0, decltype(m_args)>>) -> returns false,
  // but the following is ok:
  // (std::is_rvalue_reference_v<std::tuple_element_t<0, std::tuple<Args &&...>>>) -> returns true.
  // When emplace_callable's template parameter is deduced as tuple, inside the emplace_callable
  // it's not possible to detect rvalue ref. To bypass this issue rvalue is detected here.

  if constexpr (sizeof...(Args) == 1 && std::is_rvalue_reference_v<
    std::tuple_element_t<0, std::tuple<Args &&...>>>) {
    return dispatch(key, emplace_callable<T &&>(std::forward<Args>(args)...));
  } else {
    return dispatch(key, emplace_callable(std::forward_as_tuple(args...)));
  }
}

template <typename T>
template <typename... Args>
std::pair<typename string_hash_table_t<T>::mapped_type *, bool> string_hash_table_t<T>::try_emplace(
  key_type key, Args &&... args) {
  // Note: Here there is the same story as in emplace() (see above).
  if constexpr (sizeof...(Args) == 1 && std::is_rvalue_reference_v<
    std::tuple_element_t<0, std::tuple<Args &&...>>>) {
    return dispatch(key, try_emplace_callable<T &&>(std::forward<Args>(args)...));
  } else {
    return dispatch(key, try_emplace_callable(std::forward_as_tuple(args...)));
  }
}

template <typename T>
bool string_hash_table_t<T>::erase(key_type key) {
  return dispatch(key, erase_callable());
}


/* ==TRASH==
//PP: OK: return dispatch(key, emplace_callable<Args...>(std::forward<Args>(args)...));

  template <typename... Args>
  struct emplace_callable {
    std::tuple<Args...> &&m_args;
    emplace_callable(std::tuple<Args...> &&args) :
      m_args(std::forward<std::tuple<Args...>>(args)) {}
    //WARN: emplace_callable(Args &&... args) : m_args(std::forward_as_tuple(args...)) {}
    // BAD: emplace_callable(Args &&... args) : m_args(args...) {}
    // BAD: emplace_callable(Args &&... args) : m_args(std::forward<decltype(args)>(args)...) {}
    template <typename Map>
    std::pair<mapped_type *, bool> ALWAYS_INLINE operator()(Map &map, typename Map::key_type key) {
//      auto [it, inserted] = map.emplace(std::piecewise_construct, std::forward_as_tuple(key),
//                                        std::forward_as_tuple(m_args));
//      auto [it, inserted] = map.emplace(std::piecewise_construct, std::forward_as_tuple(key),
//                                        std::forward<std::tuple<Args...>>(m_args));
      //auto [it, inserted] = map.emplace(key, std::forward<std::tuple<Args...>>(m_args));
      auto [it, inserted] = map.emplace(std::piecewise_construct, std::forward_as_tuple(key),
                                        m_args);
      return {&it->second, inserted};
    }
  };

template <typename Key, typename T>
class empty_key_map_t {
  static_assert(sizeof(Key) <= 2 * sizeof(void *),
    "There is key passing by value - assuming small size");

public:
  using key_type = Key;
  using mapped_type = T;
  using value_type = std::pair<const key_type, mapped_type>;
  using iterator = value_type *;

  iterator begin() noexcept { return empty() ? end() : &m_value; }
  iterator end() noexcept { return &m_value + 1; }

  bool empty() const noexcept { return m_empty; }
  size_t size() const noexcept { return empty() ? 0 : 1; }
  void clear() noexcept {
    if (m_empty) return;
    m_value.second = {};  // set back default constructed
    m_empty = true;
  }

  iterator find(key_type) { return empty() ? end() : begin(); }

  std::pair<iterator, bool> insert(const value_type &value) {
    bool inserted = m_empty;
    if (m_empty) {
      m_value.second = value.second;
      m_empty = false;
    }
    return {begin(), inserted};
  }

  template <typename... Args>
  std::pair<iterator, bool> emplace(Args &&... args) {
    bool inserted = m_empty;
    if (m_empty) {
      //std::make_from_tuple<mapped_type>(args);
      //m_value.second = mapped_type(std::forward<Args>(args)...);
      //m_value.second = mapped_type(std::forward_as_tuple(args));
      m_empty = false;
    }
    return {begin(), inserted};
  }

  size_t erase(key_type) {
    size_t count = size();
    clear();
    return count;
  }

private:
  value_type m_value;
  bool m_empty = true;
};

  //src:
//  struct insert_callable {
//    const mapped_type &m_value;
//    insert_callable(const mapped_type &value) : m_value(value) {}
//    template <typename Map>
//    std::pair<mapped_type *, bool> ALWAYS_INLINE operator()(Map &map, typename Map::key_type key) {
//      auto [it, inserted] = map.insert({key, m_value});
//      return {&it->second, inserted};
//    }
//  };

//  struct insert_callable_move {
//    mapped_type &&m_value;
//    insert_callable_move(mapped_type &&value) : m_value(std::forward<mapped_type>(value)) {}
//    template <typename Map>
//    std::pair<mapped_type *, bool> ALWAYS_INLINE operator()(Map &map, typename Map::key_type key) {
//      auto [it, inserted] = map.insert({key, std::forward<mapped_type>(m_value)});
//      return {&it->second, inserted};
//    }
//  };
*/
