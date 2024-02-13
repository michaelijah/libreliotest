//These two lines include doctest and the macro lets the main function in the doctest.h file to be used as the default
#define DOCTEST_CONFIG_IMPLEMENT_IN_DLL
#include <doctest/doctest.h>

#include <sstream>
#include <stdexcept>

//Grab version information
#include <libreliotest/version.hxx>
//include files that we whose implementations we want to test
#include <libreliotest/reliotest.hxx>


#undef NDEBUG
#include <cassert>

//Always compilation under the most strict warning settings
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <iostream>
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END

//Always compilation under the most strict warning settings
DOCTEST_SYMBOL_IMPORT void from_dll();

//The conditional throw captures exceptions and stringify's those exceptions
template<typename T>
static int conditional_throw(bool in, const T& ex) {
        if(in)
#ifndef DOCTEST_CONFIG_NO_EXCEPTIONS
                throw ex; // NOLINT
#else // DOCTEST_CONFIG_NO_EXCEPTIONS
        ((void)ex);
#endif // DOCTEST_CONFIG_NO_EXCEPTIONS
    return 42;
}


//This structure allows dll/so/libs to be loaded and has functionality so tests written inside dll/so/lib can be registered without warnings.
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
DOCTEST_CLANG_SUPPRESS_WARNING("-Wnonportable-system-include-path")
#include <windows.h>
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END
#ifdef _MSC_VER
#define LIB_EXTENSION ".dll"
#else // _MSC_VER
#define LIB_EXTENSION ".dll"
#endif // _MSC_VER
#else // _WIN32
#include <dlfcn.h>
#ifdef __APPLE__
#define LIB_EXTENSION ".dylib"
#else // __APPLE__
#define LIB_EXTENSION ".so"
#endif // __APPLE__
#endif // _WIN32

// set an exception translator for double
REGISTER_EXCEPTION_TRANSLATOR(double& e) {
    return doctest::String("double: ") + doctest::toString(e);
}

using namespace std;
using namespace reliotest;



void* LoadDynamicLib(const char* lib) {
#ifdef _WIN32
    HMODULE hModule = LoadLibrary(lib);
    if (!hModule) {
        throw std::runtime_error("Failed to load dynamic library " + std::string(lib) + ": " + std::to_string(GetLastError()));
    }
    return hModule;
#else // _WIN32
    void* handle = dlopen(lib, RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        throw std::runtime_error("Failed to load dynamic library " + std::string(lib) + ": " + dlerror());
    }
    return handle;
#endif // _WIN32
}

int main(int argc, char** argv) {
        from_dll();
    LoadDynamicLib("libreliotest" LIB_EXTENSION);

        doctest::Context context;

        // defaults
        //context.addFilter("test-suite-exclude", "*implementation*"); // exclude test cases with "implmentation" in their name
        context.setOption("abort-after", 5);              // stop test execution after 5 failed assertions
        context.setOption("order-by", "name");            // sort the test cases by their name
        context.setOption("success", true);                               // Include successful assertions in output
        context.setOption("duration", true);                      // Prints the time duration of each test

        context.applyCommandLine(argc, argv);

        // overrides
        context.setOption("no-breaks", true);             // don't break in the debugger when assertions fail

        int result = context.run(); // run

        if(context.shouldExit()) // important - query flags (and --exit) rely on the user doing this
                return result;          // propagate the result of the tests

        int client_stuff_return_code = 0;
        // your program - if the testing framework is integrated in your production code

        return result + client_stuff_return_code; // the result from doctest is propagated here as well
}
