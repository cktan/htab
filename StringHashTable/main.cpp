/// \file
/// \brief Application main file

#include "string_hash_table.hpp"
#include <exception>
#include <iostream>
#include <string_view>

using namespace std::string_literals;
using namespace std::string_view_literals;

/*
struct value_t {
  int n = 0;
  std::string s;
};
*/

// Tracing version
struct value_t {
  int n = 0;
  std::string s;

  value_t() { std::cerr << "default ctor()\n"; }
  value_t(int n, const std::string &s = {}) : n(n), s(s) {
    std::cerr << "user ctor()\n";
  }
  value_t(const value_t &other) : value_t(other.n, other.s) {
    std::cerr << "copy ctor()\n";
  }
  value_t &operator=(const value_t &other) {
    std::cerr << "assignment\n";
    if (this != &other) { n = other.n; s = other.s; }
    return *this;
  }
  value_t(value_t &&other) : n(std::move(other.n)), s(std::move(other.s)) {
    std::cerr << "move ctor()\n";
  }
  value_t &operator=(value_t &&other) {
    std::cerr << "move assignment\n";
    if (this != &other) { n = std::move(other.n); s = std::move(other.s); }
    return *this;
  }
  ~value_t() { std::cerr << "dtor()\n"; }

  static void *operator new(std::size_t size, void *ptr) noexcept {
    std::cerr << "placement operator new()\n";
    return ::operator new(size, ptr);
  }
};

static std::ostream& operator<<(std::ostream &stream, const value_t &obj) {
  return stream << "{n = " << obj.n << ", s = " << obj.s << '}';
}

static std::ostream& operator<<(std::ostream &stream, const value_t *obj) {
  return obj ? stream << *obj : stream << "(null)";
}

static std::ostream& operator<<(std::ostream &stream, const string_hash_key_t &obj) {
  return stream << obj.to_string_view();
}

void exec_basic();
void exec_basic() {
  string_hash_table_t<value_t> sht(100);  // preallocates for 100 elements

  // Insert element, constructing it in-place (with global placement new)
  std::pair<value_t *, bool> insertion = sht.try_emplace("Key #1", 0, "Value #1");
  std::cerr << "inserted: " << std::boolalpha << insertion.second;
  std::cerr << ", now contained value: " << *insertion.first << std::endl;

  // Try insert another element with the same key, key origin is different here
  insertion = sht.try_emplace(std::string("Key #1"), 1, "Want replace Value #1");
  std::cerr << "inserted: " << std::boolalpha << insertion.second;
  std::cerr << ", now contained value: " << *insertion.first << std::endl;

  // Lookup value
  auto key = "Key #1"sv;
  value_t *val = sht.find(key);  // returns nullptr if not contained
  std::cerr << "Lookup: " << key << " -> " << val << std::endl;

  // Erase element
  bool erased = sht.erase(key);
  std::cerr << "Key " << key << " erased: " << std::boolalpha << erased << std::endl;
  std::cerr << "size = " << sht.size() << std::endl;

  // Insert elements via lvalue and rvalue, try_emplace() eats all
  value_t lval(12312, "I want be lvalue");
  value_t rval(12312, "I want be rvalue");
  sht.try_emplace("Key #1012", lval);  // called: user ctor(), copy ctor()
  sht.try_emplace("Key #1231", std::move(rval));  // called: move ctor()

  // Process contained elements with a lambda/functor in unspecified order
  // (printing in this case)
  sht.for_each([](string_hash_key_t &&key, const value_t &val) {
    std::cerr << key << " -> " << val << std::endl;
  });

  // Clear all
  sht.clear();
}

void exec_ref();
void exec_ref() {
  //Ref. to exactly find std container behaviour:

  std::string_view key0 = ""sv;
  std::string_view key1 = "01234567"sv;
  std::string_view key2 = "0123456701234567"sv;

  value_t val1{1, "str1"};
  value_t val2{2, "str2"};

  std::unordered_map<std::string_view, value_t> map;
  map.reserve(100);

  auto try_emplace = [&](std::string_view header) {
    std::cerr << header << '\n';

    std::cerr << "map.try_emplace(key0, 1, \"123\"):\n";
    map.try_emplace(key0, 1, "123");

    std::cerr << "map.try_emplace(key1, val1):\n";
    map.try_emplace(key1, val1);

    std::cerr << "map.try_emplace(key2, std::move(val2)):\n";
    map.try_emplace(key2, std::move(val2));
  };

  try_emplace("**** try_emplace:");
  try_emplace("**** try_emplace again:");
  std::cerr << "**** Done\n";
}

void exec_test();
void exec_test() {
  static const size_t element_count = 100;
  string_hash_table_t<value_t> sht(element_count);  // preallocates for element_count elements
  std::cerr << "size = " << sht.size() << std::endl;

  // Define some keys of varied length
  // zero size key, stores in m0 as string_key0
  std::string_view key0 = ""sv;
  // [1..8] size key, stores in m1 as string_key8
  std::string_view key1 = "01234567"sv;
  std::string_view key1x = "xxxxxxxx"sv;
  // [9..16] size key, stores in m2 as string_key16
  std::string_view key2 = "0123456701234567"sv;
  std::string_view key2x = "xxxxxxxxxxxxxxxx"sv;
  // [17..24] size key, stores in m3 as string_key24
  std::string_view key3 = "012345670123456701234567"sv;
  std::string_view key3x = "xxxxxxxxxxxxxxxxxxxxxxxx"sv;
  // > 24 size key, stores in ms as string_view
  std::string_view key4 = "01234567012345670123456701234567"sv;
  std::string_view key4x = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"sv;

  // Define some values
  value_t val1{1, "str1"};
  value_t val2{2, "str2"};
  value_t val3{3, "str3"};
  value_t val4{4, "str4"};
  value_t valx{-1, "xxxx"};

  auto try_emplace = [&](std::string_view header) {
    std::cerr << header << '\n';
    std::pair<value_t *, bool> insertion;

    insertion = sht.try_emplace(key0, 0, "I'm emplaced under zero key");
    // Ref. calls: user ctor()
    // Ref. calls if contained: nothing
    std::cerr << "inserted: " << std::boolalpha << insertion.second;
    std::cerr << ", now contained value: " << *insertion.first << std::endl;

    insertion = sht.try_emplace(key1, val1);
    // Ref. calls: user ctor(), copy ctor()
    // Ref. calls if contained: nothing
    std::cerr << "inserted: " << std::boolalpha << insertion.second;
    std::cerr << ", now contained value: " << *insertion.first << std::endl;

    insertion = sht.try_emplace(key2, std::move(val2));
    // Ref. calls: move ctor()
    // Ref. calls if contained: nothing
    std::cerr << "inserted: " << std::boolalpha << insertion.second;
    std::cerr << ", now contained value: " << *insertion.first << std::endl;

    insertion = sht.try_emplace(key3, val3);
    // Ref. calls: user ctor(), copy ctor()
    // Ref. calls if contained: nothing
    std::cerr << "inserted: " << std::boolalpha << insertion.second;
    std::cerr << ", now contained value: " << *insertion.first << std::endl;

    insertion = sht.try_emplace(key4, val4);
    // Ref. calls: user ctor(), copy ctor()
    // Ref. calls if contained: nothing
    std::cerr << "inserted: " << std::boolalpha << insertion.second;
    std::cerr << ", now contained value: " << *insertion.first << std::endl;
  };

  try_emplace("**** try_emplace:");
  try_emplace("**** try_emplace again:");

  // Find values

  std::cerr << "**** Find values:\n";
  value_t *val;

  val = sht.find(key0);
  std::cerr << std::boolalpha << "key0 -> " << val << std::endl;

  val = sht.find(key1);
  std::cerr << std::boolalpha << "key1 -> " << val << std::endl;

  val = sht.find(key2);
  std::cerr << std::boolalpha << "key2 -> " << val << std::endl;

  val = sht.find(key3);
  std::cerr << std::boolalpha << "key3 -> " << val << std::endl;

  val = sht.find(key4);
  std::cerr << std::boolalpha << "key4 -> " << val << std::endl;

  val = sht.find(key1x);
  std::cerr << std::boolalpha << "key1x -> " << val << std::endl;

  val = sht.find(key2x);
  std::cerr << std::boolalpha << "key2x -> " << val << std::endl;

  val = sht.find(key3x);
  std::cerr << std::boolalpha << "key3x -> " << val << std::endl;

  val = sht.find(key4x);
  std::cerr << std::boolalpha << "key4x -> " << val << std::endl;

  // Erase some values

  std::cerr << "**** Erase some values:\n";
  sht.erase(key0);
  sht.erase(key4);
  std::cerr << "size = " << sht.size() << std::endl;

  // Print values

  std::cerr << "**** Print values:\n";
  sht.for_each([](string_hash_key_t &&key, const value_t &val) {
    std::cerr << key << " -> " << val << std::endl;
  });

  // Clear

  std::cerr << "**** Clear:\n";
  sht.clear();
  std::cerr << "size = " << sht.size() << std::endl;

  std::cerr << "**** Done\n";
}

int main(int /*argc*/, char */*argv*/[]) {
  try {
    exec_basic();
    //exec_ref();
    //exec_test();
    return 0;
  }
  catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  catch (...) {
    std::cerr << "Unknown application error" << std::endl;
  }

  return -1;
}


/* ==TRASH==
*/
