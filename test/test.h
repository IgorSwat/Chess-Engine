#pragma once

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <functional>
#include <string>
#include <tuple>
#include <vector>


/*
    ---------- Test ----------

    This file is a root for all tests performed inside the project.
    - There are two types of tests - exact tests with asserts and some expected values,
      and general tests which main role is to print some important information or measure performance results
    - Tests are created with REGISTER_TEST macro and stored in global vector object
*/

namespace Testing {

    // ---------------
	// Test definition
	// ---------------

    // Test is a pair containing test name and test function (simple void function)
    using Test = std::pair<std::string, std::function<bool()>>;


    // ----------------
	// Global test list
	// ----------------

    // Global list that aggregates all tests created with REGISTER_TEST macro
    extern std::vector<Test> tests;


    // -------------
	// Test creation
	// -------------

    // This function allows for compile-time creating and aggregating tests
    #define REGISTER_TEST(name) \
        bool name(); \
        struct name##_t { \
            name##_t() { tests.emplace_back(#name, name); } \
        }; \
        static name##_t name##_instance; \
        bool name()
    

    // -------------------
	// Test error printing
	// -------------------

    // Color defines
    // - Create more readable UI during test run
    // - Uses ANSI macros for displaying colors in console
    #define COLOR_RESET   "\033[0m"
    #define COLOR_RED     "\033[31m"
    #define COLOR_GREEN   "\033[32m"
    #define COLOR_YELLOW  "\033[33m"
    #define COLOR_BLUE    "\033[34m"

    // A custom ASSERT macro combines the effect of standard assert and printing the error message
    // - Prints (msg) message if condition fails
    #define ASSERT_EQUALS(expected, real) \
        do { \
            if (expected != real) { \
                std::cerr << COLOR_RED "Test failed: " COLOR_RESET; \
                std::cerr << "File: " << __FILE__ << ", Line: " << __LINE__ << " - "; \
                std::cerr << "expected " << expected << ", got " << real << "\n"; \
                return false; \
            } \
        } while (0)


    // ------------
	// Test running
	// ------------

    // Perform all the tests
    // - Returns true if all tests have succeeded and false if even one test went wrong
    bool run_tests();


    // ----------------------------
	// Special tests - declarations
	// ----------------------------

    void search_speed_test(int8_t depth);

}