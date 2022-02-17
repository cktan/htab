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
    std::string_view sr0(s0, sizeof(s0) - 1);
    std::string_view sr1(s1, sizeof(s1) - 1);
    std::string_view sr2(s2, sizeof(s2) - 1);
    std::string_view sr3(s3, sizeof(s3) - 1);
    std::string_view sr4(s4, sizeof(s4) - 1);

    bool inserted;

    // Insert values
    std::cout << "Insert values:\n";

    inserted = sht.insert(sr0);
    std::cout << std::boolalpha << "sr0 inserted = " << inserted << std::endl;

    inserted = sht.insert(sr1);
    std::cout << std::boolalpha << "sr1 inserted = " << inserted << std::endl;

    inserted = sht.insert(sr2);
    std::cout << std::boolalpha << "sr2 inserted = " << inserted << std::endl;

    inserted = sht.insert(sr3);
    std::cout << std::boolalpha << "sr3 inserted = " << inserted << std::endl;

    inserted = sht.insert(sr4);
    std::cout << std::boolalpha << "sr4 inserted = " << inserted << std::endl;

    std::cout << "size = " << sht.size() << std::endl;

    // Insert again
    std::cout << "Insert values again:\n";

    inserted = sht.insert(sr0);
    std::cout << std::boolalpha << "sr0 inserted = " << inserted << std::endl;

    inserted = sht.insert(sr1);
    std::cout << std::boolalpha << "sr1 inserted = " << inserted << std::endl;

    inserted = sht.insert(sr2);
    std::cout << std::boolalpha << "sr2 inserted = " << inserted << std::endl;

    inserted = sht.insert(sr3);
    std::cout << std::boolalpha << "sr3 inserted = " << inserted << std::endl;

    inserted = sht.insert(sr4);
    std::cout << std::boolalpha << "sr4 inserted = " << inserted << std::endl;

    // Find values

    std::cout << "Find values:\n";
    bool contained;

    contained = sht.contains(sr0);
    std::cout << std::boolalpha << "sr0 contained = " << contained << std::endl;

    contained = sht.contains(sr1);
    std::cout << std::boolalpha << "sr1 contained = " << contained << std::endl;

    contained = sht.contains(sr2);
    std::cout << std::boolalpha << "sr2 contained = " << contained << std::endl;

    contained = sht.contains(sr3);
    std::cout << std::boolalpha << "sr3 contained = " << contained << std::endl;

    contained = sht.contains(sr4);
    std::cout << std::boolalpha << "sr4 contained = " << contained << std::endl;

    // Print values

    std::cout << "Print values:\n";
    sht.for_each([](string_hash_table_t::value_type v) {
      std::cout << '"' << v << '"' << std::endl;
    });

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
*/
