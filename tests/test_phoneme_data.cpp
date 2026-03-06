// test_phoneme_data.cpp — Tests for phoneme data constants
#define TEST_BUILD
#include "../formant.cpp"
#include "../test_framework.h"

// --- Test 1: kVowelA ---
REGISTER_TEST(kVowelA_params) {
    ASSERT_EQ(kVowelA.f1, 800.0);
    ASSERT_EQ(kVowelA.f2, 1200.0);
    ASSERT_EQ(kVowelA.f3, 2600.0);
    ASSERT_EQ(kVowelA.bw1, 80.0);
    ASSERT_EQ(kVowelA.bw2, 100.0);
    ASSERT_EQ(kVowelA.bw3, 120.0);
    ASSERT_EQ(kVowelA.gain, 1.0);
    ASSERT_TRUE(kVowelA.source == SourceType::Impulse);
}

// --- Test 2: kVowelI ---
REGISTER_TEST(kVowelI_params) {
    ASSERT_EQ(kVowelI.f1, 300.0);
    ASSERT_EQ(kVowelI.f2, 2300.0);
    ASSERT_EQ(kVowelI.f3, 3000.0);
    ASSERT_EQ(kVowelI.bw1, 80.0);
    ASSERT_EQ(kVowelI.bw2, 120.0);
    ASSERT_EQ(kVowelI.bw3, 150.0);
}

// --- Test 3: kVowelU ---
REGISTER_TEST(kVowelU_params) {
    ASSERT_EQ(kVowelU.f1, 350.0);
    ASSERT_EQ(kVowelU.f2, 1300.0);
    ASSERT_EQ(kVowelU.f3, 2500.0);
}

// --- Test 4: kVowelE ---
REGISTER_TEST(kVowelE_params) {
    ASSERT_EQ(kVowelE.f1, 500.0);
    ASSERT_EQ(kVowelE.f2, 1900.0);
    ASSERT_EQ(kVowelE.f3, 2600.0);
}

// --- Test 5: kVowelO ---
REGISTER_TEST(kVowelO_params) {
    ASSERT_EQ(kVowelO.f1, 500.0);
    ASSERT_EQ(kVowelO.f2, 800.0);
    ASSERT_EQ(kVowelO.f3, 2400.0);
}

// --- Test 6: All vowels have gain=1.0 and source=Impulse ---
REGISTER_TEST(all_vowels_gain_and_source) {
    const FormantParams* vowels[] = {&kVowelA, &kVowelI, &kVowelU, &kVowelE, &kVowelO};
    for (auto* v : vowels) {
        ASSERT_EQ(v->gain, 1.0);
        ASSERT_TRUE(v->source == SourceType::Impulse);
    }
}

// --- Test 7: Nasals (M, N): source=Impulse, gain=0.3 ---
REGISTER_TEST(nasal_params) {
    ASSERT_TRUE(kNasalM.source == SourceType::Impulse);
    ASSERT_EQ(kNasalM.gain, 0.3);
    ASSERT_TRUE(kNasalN.source == SourceType::Impulse);
    ASSERT_EQ(kNasalN.gain, 0.3);
}

// --- Test 8: Fricatives (H_A, S): source=Noise ---
REGISTER_TEST(fricative_source) {
    ASSERT_TRUE(kFricH_A.source == SourceType::Noise);
    ASSERT_TRUE(kFricS.source == SourceType::Noise);
}

// --- Test 9: kSilence: gain=0.0 ---
REGISTER_TEST(silence_gain) {
    ASSERT_EQ(kSilence.gain, 0.0);
}

// --- Test 10: kVoiceBar: gain=0.08, source=Impulse ---
REGISTER_TEST(voicebar_params) {
    ASSERT_EQ(kVoiceBar.gain, 0.08);
    ASSERT_TRUE(kVoiceBar.source == SourceType::Impulse);
}

// --- Test 11: Bursts (P, T, K_A): source=Noise ---
REGISTER_TEST(burst_source) {
    ASSERT_TRUE(kBurstP.source == SourceType::Noise);
    ASSERT_TRUE(kBurstT.source == SourceType::Noise);
    ASSERT_TRUE(kBurstK_A.source == SourceType::Noise);
}

// --- Test 12: kSokuonSamples == 5292 (120ms) ---
REGISTER_TEST(sokuon_samples) {
    ASSERT_EQ(kSokuonSamples, 5292);
}

// --- Test 13: kHatsuonSamples == 3528 (80ms) ---
REGISTER_TEST(hatsuon_samples) {
    ASSERT_EQ(kHatsuonSamples, 3528);
}

int main() {
    return run_all_tests("PhonemeData");
}
