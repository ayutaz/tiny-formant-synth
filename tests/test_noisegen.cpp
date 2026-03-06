#define TEST_BUILD
#include "../formant.cpp"
#include "../test_framework.h"

REGISTER_TEST(output_range) {
    NoiseGen ng;
    for (int i = 0; i < 10000; ++i) {
        double v = ng.next();
        ASSERT_GE(v, -1.0);
        ASSERT_LE(v, 1.0);
    }
}

REGISTER_TEST(deterministic) {
    NoiseGen a;
    NoiseGen b;
    for (int i = 0; i < 100; ++i) {
        ASSERT_EQ(a.next(), b.next());
    }
}

REGISTER_TEST(different_seed_different_output) {
    NoiseGen a;
    NoiseGen b;
    b.seed = 12345;
    bool any_differ = false;
    for (int i = 0; i < 100; ++i) {
        if (a.next() != b.next()) {
            any_differ = true;
            break;
        }
    }
    ASSERT_TRUE(any_differ);
}

REGISTER_TEST(mean_near_zero) {
    NoiseGen ng;
    double sum = 0.0;
    constexpr int N = 10000;
    for (int i = 0; i < N; ++i) {
        sum += ng.next();
    }
    double mean = sum / N;
    ASSERT_NEAR(mean, 0.0, 0.1);
}

REGISTER_TEST(variance_uniform) {
    NoiseGen ng;
    double sum = 0.0, sum2 = 0.0;
    constexpr int N = 10000;
    for (int i = 0; i < N; ++i) {
        double v = ng.next();
        sum += v;
        sum2 += v * v;
    }
    double mean = sum / N;
    double variance = sum2 / N - mean * mean;
    // Uniform[-1,1] has variance 1/3 ~ 0.333
    ASSERT_NEAR(variance, 1.0 / 3.0, 0.1);
}

int main() {
    return run_all_tests("NoiseGen");
}
