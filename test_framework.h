// test_framework.h — Minimal test framework (no external deps)
#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <functional>
#include <stdexcept>

#define ASSERT_TRUE(expr) \
    do { if (!(expr)) { \
        std::cerr << "  FAIL: " << #expr << " (line " << __LINE__ << ")\n"; \
        throw std::runtime_error("assertion failed"); \
    }} while(0)

#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))

#define ASSERT_EQ(a, b) \
    do { auto _a = (a); auto _b = (b); if (_a != _b) { \
        std::cerr << "  FAIL: " << #a << " == " << #b \
                  << " (" << _a << " != " << _b << ") line " << __LINE__ << "\n"; \
        throw std::runtime_error("assertion failed"); \
    }} while(0)

#define ASSERT_NEAR(a, b, eps) \
    do { auto _a = (a); auto _b = (b); if (std::abs(_a - _b) > (eps)) { \
        std::cerr << "  FAIL: |" << #a << " - " << #b \
                  << "| <= " << (eps) << " (" << _a << " vs " << _b << ") line " << __LINE__ << "\n"; \
        throw std::runtime_error("assertion failed"); \
    }} while(0)

#define ASSERT_GT(a, b) \
    do { auto _a = (a); auto _b = (b); if (!(_a > _b)) { \
        std::cerr << "  FAIL: " << #a << " > " << #b \
                  << " (" << _a << " <= " << _b << ") line " << __LINE__ << "\n"; \
        throw std::runtime_error("assertion failed"); \
    }} while(0)

#define ASSERT_LT(a, b) \
    do { auto _a = (a); auto _b = (b); if (!(_a < _b)) { \
        std::cerr << "  FAIL: " << #a << " < " << #b \
                  << " (" << _a << " >= " << _b << ") line " << __LINE__ << "\n"; \
        throw std::runtime_error("assertion failed"); \
    }} while(0)

#define ASSERT_GE(a, b) \
    do { auto _a = (a); auto _b = (b); if (!(_a >= _b)) { \
        std::cerr << "  FAIL: " << #a << " >= " << #b \
                  << " (" << _a << " < " << _b << ") line " << __LINE__ << "\n"; \
        throw std::runtime_error("assertion failed"); \
    }} while(0)

#define ASSERT_LE(a, b) \
    do { auto _a = (a); auto _b = (b); if (!(_a <= _b)) { \
        std::cerr << "  FAIL: " << #a << " <= " << #b \
                  << " (" << _a << " > " << _b << ") line " << __LINE__ << "\n"; \
        throw std::runtime_error("assertion failed"); \
    }} while(0)

// Test registration via function pointers
struct TestCase {
    std::string name;
    std::function<void()> func;
};

static std::vector<TestCase>& test_registry() {
    static std::vector<TestCase> reg;
    return reg;
}

#define REGISTER_TEST(name) \
    static void test_##name(); \
    namespace { struct AutoReg_##name { \
        AutoReg_##name() { test_registry().push_back({#name, test_##name}); } \
    } autoreg_##name; } \
    static void test_##name()

static int run_all_tests(const char* suite_name) {
    int pass = 0, fail = 0;
    std::cout << "=== " << suite_name << " ===\n";
    for (auto& tc : test_registry()) {
        try {
            tc.func();
            std::cout << "  PASS: " << tc.name << "\n";
            ++pass;
        } catch (...) {
            std::cout << "  FAIL: " << tc.name << "\n";
            ++fail;
        }
    }
    std::cout << "--- " << pass << " passed, " << fail << " failed ---\n\n";
    return fail;
}
