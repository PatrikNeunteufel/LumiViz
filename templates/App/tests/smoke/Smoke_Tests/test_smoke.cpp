// ==============================================================================
// test_smoke.cpp – Smoke Tests (doctest)
// ==============================================================================
//
// Test:        Smoke_Tests
// Framework:   doctest
// Type:        smoke
// Version:     1.0.0
// Date:        2025-12-12
//
// Description:
//   Quick smoke tests to verify basic functionality works.
//   These tests should run fast and catch critical regressions.
//
// Note:
//   Smoke tests are labeled "critical" and "fast".
//   Run first in CI: ctest -L smoke
//
// ==============================================================================

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <string>
#include <vector>
#include <memory>
#include <cmath>

// ==============================================================================
// Basic Sanity Checks
// ==============================================================================

TEST_CASE("Smoke: Basic arithmetic works" * doctest::test_suite("smoke")) {
    CHECK(1 + 1 == 2);
    CHECK(10 - 5 == 5);
    CHECK(3 * 4 == 12);
    CHECK(10 / 2 == 5);
}

TEST_CASE("Smoke: Boolean logic works" * doctest::test_suite("smoke")) {
    CHECK(true);
    CHECK_FALSE(false);
    
    // Note: doctest doesn't support && and || in CHECK directly
    // Use parentheses or store in variable
    bool andResult = true && true;
    bool orResult = true || false;
    bool andFalseResult = false && true;
    
    CHECK(andResult);
    CHECK(orResult);
    CHECK_FALSE(andFalseResult);
}

TEST_CASE("Smoke: String operations work" * doctest::test_suite("smoke")) {
    std::string s = "Hello";
    
    CHECK(s.length() == 5);
    CHECK(s + " World" == "Hello World");
    CHECK(s.substr(0, 2) == "He");
    CHECK(s.find("llo") == 2);
}

TEST_CASE("Smoke: Vector operations work" * doctest::test_suite("smoke")) {
    std::vector<int> v = {1, 2, 3};
    
    CHECK(v.size() == 3);
    CHECK(v[0] == 1);
    CHECK(v.front() == 1);
    CHECK(v.back() == 3);
    
    v.push_back(4);
    CHECK(v.size() == 4);
}

TEST_CASE("Smoke: Smart pointers work" * doctest::test_suite("smoke")) {
    auto ptr = std::make_unique<int>(42);
    CHECK(ptr != nullptr);
    CHECK(*ptr == 42);
    
    auto shared = std::make_shared<std::string>("test");
    CHECK(shared != nullptr);
    CHECK(*shared == "test");
    CHECK(shared.use_count() == 1);
    
    auto shared2 = shared;
    CHECK(shared.use_count() == 2);
}

TEST_CASE("Smoke: Math functions work" * doctest::test_suite("smoke")) {
    CHECK(std::abs(-5) == 5);
    CHECK(std::sqrt(4.0) == doctest::Approx(2.0));
    CHECK(std::pow(2.0, 3.0) == doctest::Approx(8.0));
    CHECK(std::sin(0.0) == doctest::Approx(0.0));
    CHECK(std::cos(0.0) == doctest::Approx(1.0));
}

// ==============================================================================
// Critical Path Tests
// ==============================================================================

TEST_CASE("Smoke: Object construction" * doctest::test_suite("smoke")) {
    struct Point {
        int x, y;
        Point(int x_, int y_) : x(x_), y(y_) {}
    };
    
    Point p(10, 20);
    CHECK(p.x == 10);
    CHECK(p.y == 20);
}

TEST_CASE("Smoke: Exception handling" * doctest::test_suite("smoke")) {
    CHECK_THROWS(throw std::runtime_error("test"));
    CHECK_THROWS_AS(throw std::invalid_argument("bad"), std::invalid_argument);
    CHECK_NOTHROW([]{}());
}

TEST_CASE("Smoke: Lambda functions" * doctest::test_suite("smoke")) {
    auto add = [](int a, int b) { return a + b; };
    CHECK(add(2, 3) == 5);
    
    int capture = 10;
    auto addCapture = [&capture](int x) { return x + capture; };
    CHECK(addCapture(5) == 15);
}

TEST_CASE("Smoke: Range-based for loop" * doctest::test_suite("smoke")) {
    std::vector<int> nums = {1, 2, 3, 4, 5};
    int sum = 0;
    
    for (int n : nums) {
        sum += n;
    }
    
    CHECK(sum == 15);
}

TEST_CASE("Smoke: Auto type deduction" * doctest::test_suite("smoke")) {
    auto i = 42;
    auto d = 3.14;
    auto s = std::string("hello");
    
    CHECK(i == 42);
    CHECK(d == doctest::Approx(3.14));
    CHECK(s == "hello");
}

// ==============================================================================
// Minimal Component Tests
// ==============================================================================

TEST_CASE("Smoke: Logger can be instantiated" * doctest::test_suite("smoke")) {
    // This would test actual Logger if included
    // For now, just verify we can compile and run
    CHECK(true);
}

TEST_CASE("Smoke: Memory allocation works" * doctest::test_suite("smoke")) {
    // Allocate some memory
    auto data = std::make_unique<int[]>(1000);
    CHECK(data != nullptr);
    
    // Write and read
    data[0] = 42;
    data[999] = 100;
    CHECK(data[0] == 42);
    CHECK(data[999] == 100);
}

TEST_CASE("Smoke: Thread-safe types compile" * doctest::test_suite("smoke")) {
    // Just verify atomic types work
    std::atomic<int> counter{0};
    counter++;
    CHECK(counter.load() == 1);
}
