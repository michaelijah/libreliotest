#pragma once

#include <iosfwd>
#include <string>

#include <libreliotest/export.hxx>

#include "naive_solution.hxx"

namespace reliotest
{
  // Print a greeting for the specified name into the specified
  // stream. Throw std::invalid_argument if the name is empty.
  //
  LIBRELIOTEST_SYMEXPORT void
  say_hello (std::ostream&, const std::string& name);
}
