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

void exec_ref();
void exec_ref() {
  //Ref. to exactly find std container behaviour:

  std::string_view key0 = ""sv;
  std::string_view key1 = "01234567"sv;
  std::string_view key2 = "0123456701234567"sv;
  std::string_view key3 = "012345670123456701234567"sv;
  std::string_view key4 = "01234567012345670123456701234567"sv;

  value_t val1{1, "str1"};
  value_t val2{2, "str2"};
  value_t val3{3, "str3"};
  value_t val4{4, "str4"};
  value_t valx{-1, "xxxx"};

  std::unordered_map<std::string_view, value_t> map;
  map.reserve(100);

  std::unordered_map<std::string_view, value_t>::value_type mapval3(key3, val3);
  std::unordered_map<std::string_view, value_t>::value_type mapval4(key4, val4);

  auto insert = [&](std::string_view header) {
    std::cerr << header << '\n';

    std::cerr << "map.emplace(key0, 1, \"123\"):\n";
    map.emplace(std::piecewise_construct, std::forward_as_tuple(key0),
                std::forward_as_tuple(1, "123"));

    std::cerr << "map.emplace(key1, val1):\n";
    map.emplace(key1, val1);

    std::cerr << "map.emplace(key2, std::move(val2)):\n";
    map.emplace(key2, std::move(val2));

    std::cerr << "map.insert(mapval3):\n";
    map.insert(mapval3);

    std::cerr << "map.insert(std::move(mapval4)):\n";
    map.insert(std::move(mapval4));

    std::cerr << "**** Done\n";
  };

  auto try_emplace = [&](std::string_view header) {
    std::cerr << header << '\n';

    std::cerr << "map.try_emplace(key0, 1, \"123\"):\n";
    map.try_emplace(key0, 1, "123");

    std::cerr << "map.try_emplace(key1, val1):\n";
    map.try_emplace(key1, val1);

    std::cerr << "map.try_emplace(key2, std::move(val2)):\n";
    map.try_emplace(key2, std::move(val2));
  };

  insert("**** emplace/insert:");
  insert("**** emplace/insert again:");
  map.clear();
  try_emplace("**** try_emplace:");
  try_emplace("**** try_emplace again:");
  std::cerr << "**** Done\n";
}

static void exec() {
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

  auto insert = [&](std::string_view header) {
    std::cerr << header << '\n';
    std::pair<value_t *, bool> insertion;

    insertion = sht.emplace(key0, 0, "I'm emplaced under zero key");
    // Ref. calls: user ctor()
    // Ref. calls if contained: user ctor(), dtor()
    std::cerr << "inserted: " << std::boolalpha << insertion.second;
    std::cerr << ", now contained value: " << *insertion.first << std::endl;

    insertion = sht.emplace(key1, val1);
    // Ref. calls: user ctor(), copy ctor()
    // Ref. calls: user ctor(), copy ctor(), dtor()
    std::cerr << "inserted: " << std::boolalpha << insertion.second;
    std::cerr << ", now contained value: " << *insertion.first << std::endl;

    insertion = sht.emplace(key2, std::move(val2));
    // Ref. calls: move ctor()
    // Ref. calls: move ctor(), dtor()
    std::cerr << "inserted: " << std::boolalpha << insertion.second;
    std::cerr << ", now contained value: " << *insertion.first << std::endl;

    insertion = sht.insert(key3, val3);
    // Ref. calls: user ctor(), copy ctor()
    // Ref. calls: user ctor(), copy ctor(), dtor()
    std::cerr << "inserted: " << std::boolalpha << insertion.second;
    std::cerr << ", now contained value: " << *insertion.first << std::endl;

    insertion = sht.insert(key4, std::move(val4));
    // Ref. calls: move ctor()
    // Ref. calls: move ctor(), dtor()
    std::cerr << "inserted: " << std::boolalpha << insertion.second;
    std::cerr << ", now contained value: " << *insertion.first << std::endl;
  };

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
  };

  insert("**** emplace/insert:");
  insert("**** emplace/insert again:");
  sht.clear();
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
  sht.for_each([](std::string_view key, const value_t &val) {
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
    //exec_ref();
    exec();
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
    //Ref. to exactly find std container behaviour:
    std::unordered_map<std::string_view, value_t> map;
    map.reserve(100);

    std::unordered_map<std::string_view, value_t>::value_type mapval3(key3, val3);
    std::unordered_map<std::string_view, value_t>::value_type mapval4(key4, val4);

    // Insert

    std::cerr << "map.emplace(key0, 1, \"123\"):\n";
    map.emplace(std::piecewise_construct, std::forward_as_tuple(key0),
                std::forward_as_tuple(1, "123"));

    std::cerr << "map.emplace(key1, val1):\n";
    map.emplace(key1, val1);

    std::cerr << "map.emplace(key2, std::move(val2)):\n";
    map.emplace(key2, std::move(val2));

    std::cerr << "map.insert(mapval3):\n";
    map.insert(mapval3);

    std::cerr << "map.insert(std::move(mapval4)):\n";
    map.insert(std::move(mapval4));

    // Insert again

    std::cerr << "map.emplace(key0, 1, \"123\"):\n";
    map.emplace(std::piecewise_construct, std::forward_as_tuple(key0),
                std::forward_as_tuple(1, "123"));

    std::cerr << "map.emplace(key1, val1):\n";
    map.emplace(key1, val1);

    std::cerr << "map.emplace(key2, std::move(val2)):\n";
    map.emplace(key2, std::move(val2));

    std::cerr << "map.insert(mapval3):\n";
    map.insert(mapval3);

    std::cerr << "map.insert(std::move(mapval4)):\n";
    map.insert(std::move(mapval4));

    return 0;
*/
