/// \file
/// \brief String hash table

#include <cstring>
#include <string>
#include <string_view>
#include <unordered_set>
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

template <typename T>
class empty_value_hash_table_t {
public:
  using key_type = T;
  using value_type = T;
  using iterator = const T *;

  iterator begin() const { return empty() ? end() : &m_value; }
  iterator end() const { return &m_value + 1; }

  bool empty() const { return m_empty; }
  size_t size() const { return empty() ? 0 : 1; }

  iterator find(key_type) { return empty() ? end() : begin(); }
  std::pair<iterator, bool> insert(value_type) {
    bool inserted = m_empty;
    if (m_empty) {
      m_empty = false;
    }
    return {begin(), inserted};
  }
  size_t erase(key_type) {
    size_t count = size();
    if (!m_empty) {
      m_empty = true;
    }
    return count;
  }

private:
  inline static const T m_value;
  bool m_empty = true;
};

} // detail::

class string_hash_table_t {
  // This class isn't a STL compatible container, because internally it uses
  // containers of different types to maintain varied lenght strings.So, there
  // is no unified iterator exists, and in public interface we don't use
  // iterators at all.

public:
  string_hash_table_t() {}
  string_hash_table_t(size_t min_bucket_count)
    : m0()
    , m1(min_bucket_count / 4)
    , m2(min_bucket_count / 4)
    , m3(min_bucket_count / 4)
    , ms(min_bucket_count / 4)
  {
  }

  using value_type = std::string_view;

  bool empty() const {
    return m0.empty() && m1.empty() && m2.empty() && m3.empty() && ms.empty();
  }
  size_t size() const {
    return m0.size() + m1.size() + m2.size() + m3.size() + ms.size();
  }

  inline bool ALWAYS_INLINE contains(value_type v);
  inline bool ALWAYS_INLINE insert(value_type v);
  inline bool ALWAYS_INLINE erase(value_type v);

  template<typename F>
  void for_each(F f);

private:
  detail::empty_value_hash_table_t<detail::string_key0> m0;
  std::unordered_set<detail::string_key8,  detail::hasher_t> m1;
  std::unordered_set<detail::string_key16, detail::hasher_t> m2;
  std::unordered_set<detail::string_key24, detail::hasher_t> m3;
  std::unordered_set<std::string_view,     detail::hasher_t> ms;

  template <typename Func>
  inline decltype(auto) ALWAYS_INLINE dispatch(value_type x, Func func);

  struct find_callable {
    template <typename Map>
    bool ALWAYS_INLINE operator()(Map &map, const typename Map::key_type &key) {
      return map.end() != map.find(key);
    }
  };

  struct insert_callable {
    template <typename Map>
    bool ALWAYS_INLINE operator()(Map &map, const typename Map::key_type &key) {
      auto [it, inserted] = map.insert(key);
      return inserted;
    }
  };

  struct erase_callable {
    template <typename Map>
    bool ALWAYS_INLINE operator()(Map &map, const typename Map::key_type &key) {
      return 0 != map.erase(key);
    }
  };
};

template <typename Func>
decltype(auto) string_hash_table_t::dispatch(value_type x, Func func) {
  // Dispatch is written in a way that maximizes the performance:
  // 1. Always memcpy 8 times bytes
  // 2. Use switch case extension to generate fast dispatching table
  // 3. [DELETED] Combine hash computation along with key loading
  // 4. Funcs are named callables that can be force_inlined
  // NOTE: It relies on Little Endianness and SSE4.2
  static constexpr detail::string_key0 key0;
  size_t sz = std::size(x);
  const char *p = std::data(x);
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
        const char *lp = std::data(x) + std::size(x) - 8;
        memcpy(&n[0], lp, 8);
        n[0] >>= s;
      }
      return func(m1, k8);
    }
    CASE_9_16 : {
      memcpy(&n[0], p, 8);
      const char *lp = std::data(x) + std::size(x) - 8;
      memcpy(&n[1], lp, 8);
      n[1] >>= s;
      return func(m2, k16);
    }
    CASE_17_24 : {
      memcpy(&n[0], p, 16);
      const char *lp = std::data(x) + std::size(x) - 8;
      memcpy(&n[2], lp, 8);
      n[2] >>= s;
      return func(m3, k24);
    }
    default: {
      return func(ms, x);
    }
  }
}

bool string_hash_table_t::contains(value_type v) {
  return dispatch(v, find_callable());
}

bool string_hash_table_t::insert(value_type v) {
  return dispatch(v, insert_callable());
}

bool string_hash_table_t::erase(value_type v) {
  return dispatch(v, erase_callable());
}

template<typename F>
void string_hash_table_t::for_each(F f) {
  for (auto item : m0) {
    f(detail::to_string_view(item));
  }
  for (auto item : m1) {
    f(detail::to_string_view(item));
  }
  for (auto item : m2) {
    f(detail::to_string_view(item));
  }
  for (auto item : m3) {
    f(detail::to_string_view(item));
  }
  for (auto item : ms) {
    f(detail::to_string_view(item));
  }
}


/* ==TRASH==
*/
