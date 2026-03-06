// test_new_fricatives.cpp — Tests for new fricative FormantParams constants
#define TEST_BUILD
#include "../formant.cpp"
#include "../test_framework.h"

// --- Test 1: kFricSh ---
REGISTER_TEST(kFricSh_params) {
    ASSERT_EQ(kFricSh.f1, 200.0);
    ASSERT_EQ(kFricSh.f2, 3800.0);
    ASSERT_EQ(kFricSh.f3, 6000.0);
    ASSERT_EQ(kFricSh.bw1, 500.0);
    ASSERT_EQ(kFricSh.bw2, 2500.0);
    ASSERT_EQ(kFricSh.bw3, 2000.0);
    ASSERT_EQ(kFricSh.gain, 0.4);
    ASSERT_TRUE(kFricSh.source == SourceType::Noise);
}

// --- Test 2: kFricChi ---
REGISTER_TEST(kFricChi_params) {
    ASSERT_EQ(kFricChi.f1, 200.0);
    ASSERT_EQ(kFricChi.f2, 3000.0);
    ASSERT_EQ(kFricChi.f3, 5000.0);
    ASSERT_EQ(kFricChi.bw1, 500.0);
    ASSERT_EQ(kFricChi.bw2, 2000.0);
    ASSERT_EQ(kFricChi.bw3, 2000.0);
    ASSERT_EQ(kFricChi.gain, 0.3);
    ASSERT_TRUE(kFricChi.source == SourceType::Noise);
}

// --- Test 3: kFricPhi ---
REGISTER_TEST(kFricPhi_params) {
    ASSERT_EQ(kFricPhi.f1, 200.0);
    ASSERT_EQ(kFricPhi.f2, 2500.0);
    ASSERT_EQ(kFricPhi.f3, 4000.0);
    ASSERT_EQ(kFricPhi.bw1, 500.0);
    ASSERT_EQ(kFricPhi.bw2, 3000.0);
    ASSERT_EQ(kFricPhi.bw3, 2000.0);
    ASSERT_EQ(kFricPhi.gain, 0.15);
    ASSERT_TRUE(kFricPhi.source == SourceType::Noise);
}

// --- Test 4: kFricH_E ---
REGISTER_TEST(kFricH_E_params) {
    ASSERT_EQ(kFricH_E.f1, 500.0);
    ASSERT_EQ(kFricH_E.f2, 1900.0);
    ASSERT_EQ(kFricH_E.f3, 2600.0);
    ASSERT_EQ(kFricH_E.bw1, 200.0);
    ASSERT_EQ(kFricH_E.bw2, 300.0);
    ASSERT_EQ(kFricH_E.bw3, 400.0);
    ASSERT_EQ(kFricH_E.gain, 0.15);
    ASSERT_TRUE(kFricH_E.source == SourceType::Noise);
}

// --- Test 5: kFricH_O ---
REGISTER_TEST(kFricH_O_params) {
    ASSERT_EQ(kFricH_O.f1, 500.0);
    ASSERT_EQ(kFricH_O.f2, 800.0);
    ASSERT_EQ(kFricH_O.f3, 2400.0);
    ASSERT_EQ(kFricH_O.bw1, 200.0);
    ASSERT_EQ(kFricH_O.bw2, 300.0);
    ASSERT_EQ(kFricH_O.bw3, 400.0);
    ASSERT_EQ(kFricH_O.gain, 0.15);
    ASSERT_TRUE(kFricH_O.source == SourceType::Noise);
}

// --- Test 6: kFricZ ---
REGISTER_TEST(kFricZ_params) {
    ASSERT_EQ(kFricZ.f1, 200.0);
    ASSERT_EQ(kFricZ.f2, 5500.0);
    ASSERT_EQ(kFricZ.f3, 7500.0);
    ASSERT_EQ(kFricZ.bw1, 500.0);
    ASSERT_EQ(kFricZ.bw2, 3000.0);
    ASSERT_EQ(kFricZ.bw3, 2000.0);
    ASSERT_EQ(kFricZ.gain, 0.3);
    ASSERT_TRUE(kFricZ.source == SourceType::Mixed);
}

// --- Test 7: kFricZh ---
REGISTER_TEST(kFricZh_params) {
    ASSERT_EQ(kFricZh.f1, 200.0);
    ASSERT_EQ(kFricZh.f2, 3800.0);
    ASSERT_EQ(kFricZh.f3, 6000.0);
    ASSERT_EQ(kFricZh.bw1, 500.0);
    ASSERT_EQ(kFricZh.bw2, 2500.0);
    ASSERT_EQ(kFricZh.bw3, 2000.0);
    ASSERT_EQ(kFricZh.gain, 0.3);
    ASSERT_TRUE(kFricZh.source == SourceType::Mixed);
}

// --- Test 8: /sh/ has lower F2 than /s/ ---
REGISTER_TEST(sh_lower_than_s) {
    ASSERT_LT(kFricSh.f2, kFricS.f2);
}

// --- Test 9: kFricChi is Noise source ---
REGISTER_TEST(chi_is_noise) {
    ASSERT_TRUE(kFricChi.source == SourceType::Noise);
}

// --- Test 10: kFricZ and kFricZh are Mixed source ---
REGISTER_TEST(z_zh_are_mixed) {
    ASSERT_TRUE(kFricZ.source == SourceType::Mixed);
    ASSERT_TRUE(kFricZh.source == SourceType::Mixed);
}

int main() {
    return run_all_tests("NewFricatives");
}
