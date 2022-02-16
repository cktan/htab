/// \file
/// \brief Application main file

#include "string_hash_table.hpp"
#include <exception>
#include <iostream>
#include <unordered_set>

int main(int /*argc*/, char */*argv*/[]) {
  try {
    static const size_t min_bucket_count = 20;
    StringHashTable sht(min_bucket_count);
    std::cout << "sht.size() = " << sht.size() << std::endl;

    const char str0[] = "str0";
    StringRef sr0(str0, sizeof(str0));

    const char str1[] = "str1";
    StringRef sr1(str1, sizeof(str1));

    {
      auto [where, inserted] = sht.emplace(sr0);  // or emplace -> insert
      std::cout << std::boolalpha << "sr0 inserted = " << inserted << std::endl;
      std::cout << "sht.size() = " << sht.size() << std::endl;
    }
    {
      auto [where, inserted] = sht.insert(sr0);
      std::cout << std::boolalpha << "sr0 inserted = " << inserted << std::endl;
      std::cout << "sht.size() = " << sht.size() << std::endl;
    }
    {
      StringHashTable::iterator where = sht.find(sr0);
      bool contained = sht.end() != where;
      std::cout << std::boolalpha << "sr0 contained = " << contained << std::endl;
    }
    {
      StringHashTable::iterator where = sht.find(sr1);
      bool contained = sht.end() != where;
      std::cout << std::boolalpha << "sr1 contained = " << contained << std::endl;
    }

    {
      StringRef empty;
      auto [where, inserted] = sht.insert(empty);
      std::cout << std::boolalpha << "empty inserted = " << inserted << std::endl;
      std::cout << "sht.size() = " << sht.size() << std::endl;
    }
    {
      StringRef empty;
      auto [where, inserted] = sht.insert(empty);
      std::cout << std::boolalpha << "empty inserted = " << inserted << std::endl;
      std::cout << "sht.size() = " << sht.size() << std::endl;
    }

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
//#include <Common/HashTable/StringHashTable.h>
    std::cout << "sizeof(std::string) = " << sizeof(std::string) << std::endl;
    std::cout << "sizeof(std::string_view) = " << sizeof(std::string_view) << std::endl;
std::cout << "sizeof(StringKey0) = " << sizeof(StringKey0) << std::endl;
*/
