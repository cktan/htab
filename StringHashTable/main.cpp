/// \file
/// \brief Application main file

#include <Common/HashTable/StringHashTable.h>
#include <exception>
#include <iostream>

int main(int /*argc*/, char */*argv*/[]) {
  try {
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
//using namespace std::string_literals;
//#include <algorithm>
//#include <chrono>
//#include <cstdlib>
//#include <cstring>
//#include <unistd.h>
*/
