#pragma once

#include <iostream>
#include <functional>

struct TestCase {
    std::string name;
    std::function<bool()> test;
};

inline bool run(std::initializer_list<TestCase> tests, const char* prefix = "\n") {
    size_t success = 0;
    for (const auto& t : tests) {
        try {
            std::cout << prefix << t.name << ": ";
            const bool ok = t.test();
            success += ok;
            std::cout << (ok ? " OK" : " FAIL");
        } catch (const std::exception& ex) {
            std::cout << "exception: " << ex.what();
        } catch (...) {
            std::cout << "Unknown exception";
        }
    }
    std::cout << std::endl;
    return success == tests.size();
}
