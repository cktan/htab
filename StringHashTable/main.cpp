/// \file
/// \brief Application main file

#include "string_hash_table.hpp"
#include <exception>
#include <iostream>
#include <unordered_set>

int main(int /*argc*/, char */*argv*/[]) {
  try {
    static const size_t min_bucket_count = 4;
    string_hash_table_t sht(min_bucket_count);
    std::cout << "size = " << sht.size() << std::endl;

    const char s0[] = "";
    const char s1[] = "01234567";
    const char s2[] = "0123456701234567";
    const char s3[] = "012345670123456701234567";
    const char s4[] = "01234567012345670123456701234567";
    StringRef sr0(s0, sizeof(s0));
    StringRef sr1(s1, sizeof(s1));
    StringRef sr2(s2, sizeof(s2));
    StringRef sr3(s3, sizeof(s3));
    StringRef sr4(s4, sizeof(s4));

    std::pair<string_hash_table_t::value_holder, bool> insertion;

    // Insert values
    std::cout << "Insert values:\n";

    insertion = sht.insert(sr0);
    std::cout << std::boolalpha << "sr0 inserted = " << insertion.second << std::endl;

    insertion = sht.insert(sr1);
    std::cout << std::boolalpha << "sr1 inserted = " << insertion.second << std::endl;

    insertion = sht.insert(sr2);
    std::cout << std::boolalpha << "sr2 inserted = " << insertion.second << std::endl;

    insertion = sht.insert(sr3);
    std::cout << std::boolalpha << "sr3 inserted = " << insertion.second << std::endl;

    insertion = sht.insert(sr4);
    std::cout << std::boolalpha << "sr4 inserted = " << insertion.second << std::endl;

    std::cout << "size = " << sht.size() << std::endl;

    // Insert again
    std::cout << "Insert values again:\n";

    insertion = sht.insert(sr0);
    std::cout << std::boolalpha << "sr0 inserted = " << insertion.second << std::endl;

    insertion = sht.insert(sr1);
    std::cout << std::boolalpha << "sr1 inserted = " << insertion.second << std::endl;

    insertion = sht.insert(sr2);
    std::cout << std::boolalpha << "sr2 inserted = " << insertion.second << std::endl;

    insertion = sht.insert(sr3);
    std::cout << std::boolalpha << "sr3 inserted = " << insertion.second << std::endl;

    insertion = sht.insert(sr4);
    std::cout << std::boolalpha << "sr4 inserted = " << insertion.second << std::endl;

    // Find values

    std::cout << "Find values:\n";
    string_hash_table_t::value_holder where;

    where = sht.find(sr0);
    std::cout << std::boolalpha << "sr0 contained = " << (sht.end() != where) << std::endl;

    where = sht.find(sr1);
    std::cout << std::boolalpha << "sr1 contained = " << (sht.end() != where) << std::endl;

    where = sht.find(sr2);
    std::cout << std::boolalpha << "sr2 contained = " << (sht.end() != where) << std::endl;

    where = sht.find(sr3);
    std::cout << std::boolalpha << "sr3 contained = " << (sht.end() != where) << std::endl;

    where = sht.find(sr4);
    std::cout << std::boolalpha << "sr4 contained = " << (sht.end() != where) << std::endl;

    // Erase values

    std::cout << "Erase values:\n";
    sht.erase(sr0);
    sht.erase(sr1);
    sht.erase(sr2);
    sht.erase(sr3);
    sht.erase(sr4);
    std::cout << "size = " << sht.size() << std::endl;

    string_hash_table_t sht1;
    string_hash_table_t sht2;

    sht1 = std::move(sht1);
    string_hash_table_t sht3(std::move(sht1));

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

//#include <Common/HashTable/StringHashTable.h>
    std::cout << "sizeof(std::string) = " << sizeof(std::string) << std::endl;
    std::cout << "sizeof(std::string_view) = " << sizeof(std::string_view) << std::endl;
std::cout << "sizeof(StringKey0) = " << sizeof(StringKey0) << std::endl;
*/
