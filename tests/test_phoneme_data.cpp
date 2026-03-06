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

// --- Test 14: kFricSh ---
REGISTER_TEST(kFricSh_values) {
    ASSERT_EQ(kFricSh.f1, 200.0);
    ASSERT_EQ(kFricSh.f2, 3800.0);
    ASSERT_EQ(kFricSh.f3, 6000.0);
    ASSERT_EQ(kFricSh.bw1, 500.0);
    ASSERT_EQ(kFricSh.bw2, 2500.0);
    ASSERT_EQ(kFricSh.bw3, 2000.0);
    ASSERT_EQ(kFricSh.gain, 0.4);
    ASSERT_TRUE(kFricSh.source == SourceType::Noise);
}

// --- Test 15: kFricChi ---
REGISTER_TEST(kFricChi_values) {
    ASSERT_EQ(kFricChi.f1, 200.0);
    ASSERT_EQ(kFricChi.f2, 3000.0);
    ASSERT_EQ(kFricChi.f3, 5000.0);
    ASSERT_EQ(kFricChi.bw1, 500.0);
    ASSERT_EQ(kFricChi.bw2, 2000.0);
    ASSERT_EQ(kFricChi.bw3, 2000.0);
    ASSERT_EQ(kFricChi.gain, 0.3);
    ASSERT_TRUE(kFricChi.source == SourceType::Noise);
}

// --- Test 16: kFricPhi ---
REGISTER_TEST(kFricPhi_values) {
    ASSERT_EQ(kFricPhi.f1, 200.0);
    ASSERT_EQ(kFricPhi.f2, 2500.0);
    ASSERT_EQ(kFricPhi.f3, 4000.0);
    ASSERT_EQ(kFricPhi.bw1, 500.0);
    ASSERT_EQ(kFricPhi.bw2, 3000.0);
    ASSERT_EQ(kFricPhi.bw3, 2000.0);
    ASSERT_EQ(kFricPhi.gain, 0.15);
    ASSERT_TRUE(kFricPhi.source == SourceType::Noise);
}

// --- Test 17: kFricH_E ---
REGISTER_TEST(kFricH_E_values) {
    ASSERT_EQ(kFricH_E.f1, 500.0);
    ASSERT_EQ(kFricH_E.f2, 1900.0);
    ASSERT_EQ(kFricH_E.f3, 2600.0);
    ASSERT_EQ(kFricH_E.bw1, 200.0);
    ASSERT_EQ(kFricH_E.bw2, 300.0);
    ASSERT_EQ(kFricH_E.bw3, 400.0);
    ASSERT_EQ(kFricH_E.gain, 0.15);
    ASSERT_TRUE(kFricH_E.source == SourceType::Noise);
}

// --- Test 18: kFricH_O ---
REGISTER_TEST(kFricH_O_values) {
    ASSERT_EQ(kFricH_O.f1, 500.0);
    ASSERT_EQ(kFricH_O.f2, 800.0);
    ASSERT_EQ(kFricH_O.f3, 2400.0);
    ASSERT_EQ(kFricH_O.bw1, 200.0);
    ASSERT_EQ(kFricH_O.bw2, 300.0);
    ASSERT_EQ(kFricH_O.bw3, 400.0);
    ASSERT_EQ(kFricH_O.gain, 0.15);
    ASSERT_TRUE(kFricH_O.source == SourceType::Noise);
}

// --- Test 19: kFricZ (Mixed source) ---
REGISTER_TEST(kFricZ_values) {
    ASSERT_EQ(kFricZ.f1, 200.0);
    ASSERT_EQ(kFricZ.f2, 5500.0);
    ASSERT_EQ(kFricZ.f3, 7500.0);
    ASSERT_EQ(kFricZ.bw1, 500.0);
    ASSERT_EQ(kFricZ.bw2, 3000.0);
    ASSERT_EQ(kFricZ.bw3, 2000.0);
    ASSERT_EQ(kFricZ.gain, 0.3);
    ASSERT_TRUE(kFricZ.source == SourceType::Mixed);
}

// --- Test 20: kFricZh (Mixed source) ---
REGISTER_TEST(kFricZh_values) {
    ASSERT_EQ(kFricZh.f1, 200.0);
    ASSERT_EQ(kFricZh.f2, 3800.0);
    ASSERT_EQ(kFricZh.f3, 6000.0);
    ASSERT_EQ(kFricZh.bw1, 500.0);
    ASSERT_EQ(kFricZh.bw2, 2500.0);
    ASSERT_EQ(kFricZh.bw3, 2000.0);
    ASSERT_EQ(kFricZh.gain, 0.3);
    ASSERT_TRUE(kFricZh.source == SourceType::Mixed);
}

// --- Test 21: kBurstTCh ---
REGISTER_TEST(kBurstTCh_values) {
    ASSERT_EQ(kBurstTCh.f1, 300.0);
    ASSERT_EQ(kBurstTCh.f2, 3800.0);
    ASSERT_EQ(kBurstTCh.f3, 6000.0);
    ASSERT_EQ(kBurstTCh.bw1, 500.0);
    ASSERT_EQ(kBurstTCh.bw2, 2000.0);
    ASSERT_EQ(kBurstTCh.bw3, 2000.0);
    ASSERT_EQ(kBurstTCh.gain, 0.35);
    ASSERT_TRUE(kBurstTCh.source == SourceType::Noise);
}

// --- Test 22: kBurstTs ---
REGISTER_TEST(kBurstTs_values) {
    ASSERT_EQ(kBurstTs.f1, 300.0);
    ASSERT_EQ(kBurstTs.f2, 5500.0);
    ASSERT_EQ(kBurstTs.f3, 7500.0);
    ASSERT_EQ(kBurstTs.bw1, 500.0);
    ASSERT_EQ(kBurstTs.bw2, 2500.0);
    ASSERT_EQ(kBurstTs.bw3, 2000.0);
    ASSERT_EQ(kBurstTs.gain, 0.35);
    ASSERT_TRUE(kBurstTs.source == SourceType::Noise);
}

int main() {
    return run_all_tests("PhonemeData");
}
