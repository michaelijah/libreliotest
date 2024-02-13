//DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL should appear once in each library
#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
//DOCTEST_CONFIG_IMPLEMENT should only appear in the library
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <iostream>
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END

#include <libreliotest/reliotest.hxx>

#include <ostream>
#include <stdexcept>

using namespace std;

namespace reliotest
{
  void say_hello (ostream& o, const string& n)
  {
    if (n.empty ())
      throw invalid_argument ("empty name");

    o << "Hello, " << n << '!' << endl;
  }
}

DOCTEST_SYMBOL_EXPORT void from_dll(); // to silence "-Wmissing-declarations" with GCC
DOCTEST_SYMBOL_EXPORT void from_dll() {} // force the creation of a .lib file with MSVC
