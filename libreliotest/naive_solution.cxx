#include <libreliotest/naive_solution.hxx>
#include <doctest/doctest.h>

DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <iostream>
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END

using namespace reliotest;

TEST_SUITE_BEGIN("implementation" * doctest::description("naive solution"));
TEST_CASE("Run Naive Solution")
{
    //create functor
    naivety test_object("input.txt","naive_output.txt");

    //run the algo
    test_object();
}
TEST_SUITE_END();
