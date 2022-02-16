/// \file
/// \brief String hash table

//TODO: replace StringRef with std::string_view
#include <common/StringRef.h>  // StringRef + SIMD functions for comparison
#include <cstring>
#include <string>
#include <unordered_set>
#include <smmintrin.h>

#define ALWAYS_INLINE __attribute__((__always_inline__))

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

using UInt64 = uint64_t;  //TODO: use directly uint64_t

struct StringKey0 {};
inline bool operator==(StringKey0, StringKey0) { return true; }

using StringKey8 = UInt64;

struct StringKey16
{
  UInt64 a;
  UInt64 b;

  bool operator==(const StringKey16 rhs) const { return a == rhs.a && b == rhs.b; }
  bool operator!=(const StringKey16 rhs) const { return !operator==(rhs); }
  bool operator==(const UInt64 rhs) const { return a == rhs && b == 0; }
  bool operator!=(const UInt64 rhs) const { return !operator==(rhs); }

  StringKey16 & operator=(const UInt64 rhs)
  {
    a = rhs;
    b = 0;
    return *this;
  }
};

struct StringKey24
{
  UInt64 a;
  UInt64 b;
  UInt64 c;

  bool operator==(const StringKey24 rhs) const { return a == rhs.a && b == rhs.b && c == rhs.c; }
  bool operator!=(const StringKey24 rhs) const { return !operator==(rhs); }
  bool operator==(const UInt64 rhs) const { return a == rhs && b == 0 && c == 0; }
  bool operator!=(const UInt64 rhs) const { return !operator==(rhs); }

  StringKey24 & operator=(const UInt64 rhs)
  {
    a = rhs;
    b = 0;
    c = 0;
    return *this;
  }
};

inline StringRef ALWAYS_INLINE toStringRef(const StringKey0 &) { return {}; }

inline StringRef ALWAYS_INLINE toStringRef(const StringKey8 & n) {
  return {reinterpret_cast<const char *>(&n), 8ul - (__builtin_clzll(n) >> 3)};
}

inline StringRef ALWAYS_INLINE toStringRef(const StringKey16 & n) {
  return {reinterpret_cast<const char *>(&n), 16ul - (__builtin_clzll(n.b) >> 3)};
}

inline StringRef ALWAYS_INLINE toStringRef(const StringKey24 & n) {
  return {reinterpret_cast<const char *>(&n), 24ul - (__builtin_clzll(n.c) >> 3)};
}

inline const StringRef & ALWAYS_INLINE toStringRef(const StringRef & s) {
  return s;
}

struct StringHashTableHash
{
  size_t ALWAYS_INLINE operator()(StringKey0) const { return 0; }

  size_t ALWAYS_INLINE operator()(StringKey8 key) const {
    size_t res = size_t(-1ULL);
    res = _mm_crc32_u64(res, key);
    return res;
  }

  size_t ALWAYS_INLINE operator()(StringKey16 key) const {
    size_t res = size_t(-1ULL);
    res = _mm_crc32_u64(res, key.a);
    res = _mm_crc32_u64(res, key.b);
    return res;
  }

  size_t ALWAYS_INLINE operator()(StringKey24 key) const {
    size_t res = size_t(-1ULL);
    res = _mm_crc32_u64(res, key.a);
    res = _mm_crc32_u64(res, key.b);
    res = _mm_crc32_u64(res, key.c);
    return res;
  }

  size_t ALWAYS_INLINE operator()(StringRef key) const {
    size_t res = size_t(-1ULL);
    size_t sz = key.size;
    const char * p = key.data;
    const char * lp = p + sz - 8; // starting pointer of the last 8 bytes segment
    char s = (-sz & 7) * 8; // pending bits that needs to be shifted out
    UInt64 n[3]; // StringRef in SSO map will have length > 24
    memcpy(&n, p, 24);
    res = _mm_crc32_u64(res, n[0]);
    res = _mm_crc32_u64(res, n[1]);
    res = _mm_crc32_u64(res, n[2]);
    p += 24;
    while (p + 8 < lp)
    {
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

template <typename Cell>
struct StringHashTableEmpty
{
  using Self = StringHashTableEmpty;

  Cell value;
  bool is_empty{true};

  StringHashTableEmpty() { memset(reinterpret_cast<char *>(&value), 0, sizeof(value)); }

  template <bool is_const>
  struct iterator_base
  {
    using Parent = std::conditional_t<is_const, const Self *, Self *>;
    Cell * ptr;

    friend struct iterator_base<!is_const>; // bidirectional friendliness

    iterator_base(Cell * ptr_ = nullptr) : ptr(ptr_) {}

    iterator_base & operator++()
    {
      ptr = nullptr;
      return *this;
    }

    auto & operator*() const { return *ptr; }
    auto * operator-> () const { return ptr; }

    auto getPtr() const { return ptr; }
    size_t getHash() const { return 0; }
  };
  using iterator = iterator_base<false>;
  using const_iterator = iterator_base<true>;

  friend bool operator==(const iterator & lhs, const iterator & rhs)
  {
    return (lhs.ptr == nullptr && rhs.ptr == nullptr) || (lhs.ptr != nullptr && rhs.ptr != nullptr);
  }
  friend bool operator!=(const iterator & lhs, const iterator & rhs) { return !(lhs == rhs); }
  friend bool operator==(const const_iterator & lhs, const const_iterator & rhs)
  {
    return (lhs.ptr == nullptr && rhs.ptr == nullptr) || (lhs.ptr != nullptr && rhs.ptr != nullptr);
  }
  friend bool operator!=(const const_iterator & lhs, const const_iterator & rhs) { return !(lhs == rhs); }

  std::pair<iterator, bool> ALWAYS_INLINE insert(const Cell &) {
    if (is_empty) {
      is_empty = false;
      //src: value->setMapped(x);
      return {begin(), true};
    }
    return {begin(), false};
  }

  std::pair<iterator, bool> ALWAYS_INLINE emplace(const Cell &) {
    bool inserted;
    if (is_empty) {
      inserted = true;
      is_empty = false;
    }
    else
      inserted = false;
    return {begin(), inserted};
  }

  template <typename Key>
  iterator ALWAYS_INLINE find(Key) { return begin(); }

  const_iterator begin() const {
    if (is_empty)
      return end();
    return {&value};
  }
  iterator begin() {
    if (is_empty)
      return end();
    return {&value};
  }
  const_iterator end() const { return {}; }
  iterator end() { return {}; }

  size_t size() const { return is_empty ? 0 : 1; }
  bool empty() const { return is_empty; }
  size_t getBufferSizeInBytes() const { return sizeof(Cell); }
  size_t getCollisions() const { return 0; }
};

class StringHashTable
{
private:
  typedef StringHashTableEmpty<StringKey0> T0;
  typedef std::unordered_set<StringKey8,  StringHashTableHash> T1;
  typedef std::unordered_set<StringKey16, StringHashTableHash> T2;
  typedef std::unordered_set<StringKey24, StringHashTableHash> T3;
  typedef std::unordered_set<StringRef,   StringHashTableHash> Ts;

  T0 m0;
  T1 m1;
  T2 m2;
  T3 m3;
  Ts ms;

public:
  using Key = StringRef;
  using key_type = Key;
  using value_type = typename Ts::value_type;

  StringHashTable(const StringHashTable&) = delete;
  StringHashTable& operator=(const StringHashTable&) = delete;

  StringHashTable() {}
  StringHashTable(size_t reserve_for_num_elements)
    : m0()
    , m1(reserve_for_num_elements / 4)
    , m2(reserve_for_num_elements / 4)
    , m3(reserve_for_num_elements / 4)
    , ms(reserve_for_num_elements / 4)
  {
  }

  //TODO:disabled
//  StringHashTable(StringHashTable &&rhs) { *this = std::move(rhs); }
//  ~StringHashTable() {}

  //TODO: remove ValueHolder: use StringRef directly
  struct ValueHolder
  {
    StringRef value;
    auto *operator-> () { return this; }
    value_type getValue() const { return value; }
    ValueHolder() : value{} {}
    template <typename Iterator>
    //src:ValueHolder(const Iterator & iter) : value(toStringRef(iter->getValue())) {}
    ValueHolder(const Iterator &iter) : value(toStringRef(*iter)) {}
    template <typename Iterator>
    void operator=(const Iterator &iter) {
      //src:value = toStringRef(iter->getValue());
      value = toStringRef(*iter);
    }
    // Only used to check if it's end() in find
    bool operator==(const ValueHolder &that) const { return value.size == 0 && that.value.size == 0; }
    bool operator!=(const ValueHolder &that) const { return !(*this == that); }
  };
  using iterator = ValueHolder;

  ValueHolder ALWAYS_INLINE end() { return ValueHolder{}; }
  size_t size() const { return m0.size() + m1.size() + m2.size() + m3.size() + ms.size(); }
  bool empty() const { return m0.empty() && m1.empty() && m2.empty() && m3.empty() && ms.empty(); }

  // Dispatch is written in a way that maximizes the performance:
  // 1. Always memcpy 8 times bytes
  // 2. Use switch case extension to generate fast dispatching table
  // 3. [DELETED] Combine hash computation along with key loading
  // 4. Funcs are named callables that can be force_inlined
  // NOTE: It relies on Little Endianness and SSE4.2
  template <typename Func>
  decltype(auto) ALWAYS_INLINE dispatch(Key x, Func func)
  {
    static constexpr StringKey0 key0{};
    size_t sz = x.size;
    const char * p = x.data;
    // pending bits that needs to be shifted out
    char s = (-sz & 7) * 8;
    union
    {
      StringKey8 k8;
      StringKey16 k16;
      StringKey24 k24;
      UInt64 n[3];
    };
    switch (sz)
    {
    case 0:
        return func(m0, key0);
    CASE_1_8 : {
        // first half page
        if ((reinterpret_cast<uintptr_t>(p) & 2048) == 0)
        {
          memcpy(&n[0], p, 8);
          n[0] &= -1ul >> s;
        }
        else
        {
          const char * lp = x.data + x.size - 8;
          memcpy(&n[0], lp, 8);
          n[0] >>= s;
        }
        return func(m1, k8);
      }
    CASE_9_16 : {
        memcpy(&n[0], p, 8);
        const char * lp = x.data + x.size - 8;
        memcpy(&n[1], lp, 8);
        n[1] >>= s;
        return func(m2, k16);
      }
    CASE_17_24 : {
        memcpy(&n[0], p, 16);
        const char * lp = x.data + x.size - 8;
        memcpy(&n[2], lp, 8);
        n[2] >>= s;
        return func(m3, k24);
      }
      default: {
        memcpy(&n, x.data, 24);
        p += 24;
        const char * lp = x.data + x.size - 8;
        while (p + 8 < lp)
        {
          memcpy(&n[0], p, 8);
          p += 8;
        }
        memcpy(&n[0], lp, 8);
        n[0] >>= s;
        return func(ms, x);
      }
    }
  }

  //std: iterator find ( const key_type& k );
  struct FindCallable {
    template <typename Map, typename Key>
    ValueHolder ALWAYS_INLINE operator()(Map &map, const Key &x) {
      typename Map::iterator it = map.find(x);
      return it != map.end() ? ValueHolder(it) : ValueHolder();
    }
  };
  ValueHolder ALWAYS_INLINE find(Key x) {
    return dispatch(x, FindCallable{});
  }

  //std: pair<iterator,bool> insert ( const value_type& val );
  struct InsertCallable {
    template <typename Map, typename Key>
    std::pair<ValueHolder, bool> ALWAYS_INLINE operator()(Map &map, const Key &x) {
      auto [it, inserted] = map.insert(x);
      return {it != map.end() ? ValueHolder(it) : ValueHolder(), inserted};
    }
  };
  std::pair<ValueHolder, bool> ALWAYS_INLINE insert(Key x) {
    return dispatch(x, InsertCallable{});
  }

  //std: template <class... Args> pair <iterator,bool> emplace ( Args&&... args );
  struct EmplaceCallable {
    template <typename Map, typename Key>
    std::pair<ValueHolder, bool> ALWAYS_INLINE operator()(Map &map, const Key &x) {
      auto [it, inserted] = map.emplace(x);
      return {it != map.end() ? ValueHolder(it) : ValueHolder(), inserted};
    }
  };
  std::pair<ValueHolder, bool> ALWAYS_INLINE emplace(Key x) {
    return dispatch(x, EmplaceCallable{});
  }
};


/* ==TRASH==
//template<>
//struct std::hash<StringKey0> {
//  size_t ALWAYS_INLINE operator()(StringKey0) const noexcept { return 0; }
//};

  //src:
//  struct EmplaceCallable
//  {
//    ValueHolder & it;
//    bool & inserted;
//    EmplaceCallable(ValueHolder & it_, bool & inserted_) : it(it_), inserted(inserted_) {}
//    template <typename Map, typename Key>
//    void ALWAYS_INLINE operator()(Map & map, const Key & x, size_t hash)
//    {
//      typename Map::iterator impl_it;
//      map.emplace(x, impl_it, inserted, hash);
//      it = impl_it;
//    }
//  };
//  void ALWAYS_INLINE emplace(Key x, ValueHolder & it, bool & inserted) { dispatch(x, EmplaceCallable{it, inserted}); }
*/
