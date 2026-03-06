// test_lerp.cpp — Unit tests for lerp (FormantParams linear interpolation)
#define TEST_BUILD
#include "../formant.cpp"
#include "../test_framework.h"

constexpr double kEps = 1e-9;

// 1. t=0.0 で a の値がそのまま返る
REGISTER_TEST(lerp_t0_returns_a) {
    FormantParams a = {800, 1200, 2600, 80, 100, 120, 0.8, SourceType::Impulse};
    FormantParams b = {300, 2300, 3000, 80, 120, 150, 1.0, SourceType::Noise};

    auto r = lerp(a, b, 0.0);
    ASSERT_NEAR(r.f1,   800.0, kEps);
    ASSERT_NEAR(r.f2,  1200.0, kEps);
    ASSERT_NEAR(r.f3,  2600.0, kEps);
    ASSERT_NEAR(r.bw1,   80.0, kEps);
    ASSERT_NEAR(r.bw2,  100.0, kEps);
    ASSERT_NEAR(r.bw3,  120.0, kEps);
    ASSERT_NEAR(r.gain,   0.8, kEps);
}

// 2. t=1.0 で b の値がそのまま返る
REGISTER_TEST(lerp_t1_returns_b) {
    FormantParams a = {800, 1200, 2600, 80, 100, 120, 0.8, SourceType::Impulse};
    FormantParams b = {300, 2300, 3000, 80, 120, 150, 1.0, SourceType::Noise};

    auto r = lerp(a, b, 1.0);
    ASSERT_NEAR(r.f1,   300.0, kEps);
    ASSERT_NEAR(r.f2,  2300.0, kEps);
    ASSERT_NEAR(r.f3,  3000.0, kEps);
    ASSERT_NEAR(r.bw1,   80.0, kEps);
    ASSERT_NEAR(r.bw2,  120.0, kEps);
    ASSERT_NEAR(r.bw3,  150.0, kEps);
    ASSERT_NEAR(r.gain,   1.0, kEps);
}

// 3. t=0.5 で中間値が返る
REGISTER_TEST(lerp_t05_returns_midpoint) {
    FormantParams a = {800, 1200, 2600, 80, 100, 120, 0.8, SourceType::Impulse};
    FormantParams b = {300, 2300, 3000, 80, 120, 150, 1.0, SourceType::Noise};

    auto r = lerp(a, b, 0.5);
    ASSERT_NEAR(r.f1,   550.0, kEps);
    ASSERT_NEAR(r.f2,  1750.0, kEps);
    ASSERT_NEAR(r.f3,  2800.0, kEps);
    ASSERT_NEAR(r.bw1,   80.0, kEps);
    ASSERT_NEAR(r.bw2,  110.0, kEps);
    ASSERT_NEAR(r.bw3,  135.0, kEps);
    ASSERT_NEAR(r.gain,   0.9, kEps);
}

// 4. sourceType は補間されず a.source が使われる
REGISTER_TEST(lerp_source_not_interpolated) {
    FormantParams a = {800, 1200, 2600, 80, 100, 120, 1.0, SourceType::Noise};
    FormantParams b = {300, 2300, 3000, 80, 120, 150, 1.0, SourceType::Impulse};

    auto r0 = lerp(a, b, 0.0);
    auto r5 = lerp(a, b, 0.5);
    auto r1 = lerp(a, b, 1.0);

    ASSERT_TRUE(r0.source == SourceType::Noise);
    ASSERT_TRUE(r5.source == SourceType::Noise);
    ASSERT_TRUE(r1.source == SourceType::Noise);
}

// 5. 同じパラメータ同士の lerp では値が変わらない
REGISTER_TEST(lerp_same_params_unchanged) {
    FormantParams p = {500, 1900, 2600, 80, 100, 120, 0.7, SourceType::Impulse};

    for (double t : {0.0, 0.25, 0.5, 0.75, 1.0}) {
        auto r = lerp(p, p, t);
        ASSERT_NEAR(r.f1,   500.0, kEps);
        ASSERT_NEAR(r.f2,  1900.0, kEps);
        ASSERT_NEAR(r.f3,  2600.0, kEps);
        ASSERT_NEAR(r.bw1,   80.0, kEps);
        ASSERT_NEAR(r.bw2,  100.0, kEps);
        ASSERT_NEAR(r.bw3,  120.0, kEps);
        ASSERT_NEAR(r.gain,   0.7, kEps);
    }
}

// 6. kVowelA と kVowelI の間の補間 (t=0.5) が正しい
REGISTER_TEST(lerp_vowelA_vowelI_midpoint) {
    auto r = lerp(kVowelA, kVowelI, 0.5);

    // kVowelA = {800, 1200, 2600, 80, 100, 120}  gain=1.0
    // kVowelI = {300, 2300, 3000, 80, 120, 150}  gain=1.0
    ASSERT_NEAR(r.f1,   550.0, kEps);  // (800+300)/2
    ASSERT_NEAR(r.f2,  1750.0, kEps);  // (1200+2300)/2
    ASSERT_NEAR(r.f3,  2800.0, kEps);  // (2600+3000)/2
    ASSERT_NEAR(r.bw1,   80.0, kEps);  // (80+80)/2
    ASSERT_NEAR(r.bw2,  110.0, kEps);  // (100+120)/2
    ASSERT_NEAR(r.bw3,  135.0, kEps);  // (120+150)/2
    ASSERT_NEAR(r.gain,   1.0, kEps);  // (1.0+1.0)/2
    ASSERT_TRUE(r.source == SourceType::Impulse);
}

int main() {
    return run_all_tests("Lerp");
}
