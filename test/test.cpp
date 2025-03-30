#include "test.h"
#include <iomanip>


namespace Testing {

    // -------------
	// Test runnning
	// -------------

    bool run_tests()
    {
        auto& tests = get_tests();
        std::cout << COLOR_BLUE "Running " << tests.size() << " test(s)...\n" COLOR_RESET << std::string(40, '-') << "\n";

        bool result = true;

        // Perform every test once
        for (const auto& test : tests) {
            std::cout << COLOR_BLUE "Running " << test.first << "..." COLOR_RESET "\n";

            if (test.second())
                std::cout << COLOR_GREEN "Test passed!\n" COLOR_RESET;
            else
                result = false;
        }

        std::cout << std::string(40, '-') << "\n" << COLOR_BLUE "All tests finished.\n" COLOR_RESET;

        return result;
    }

}