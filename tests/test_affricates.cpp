// test_affricates.cpp — Unit tests for affricate consonants (/tɕ/, /ts/, /dʑ/, /dz/)
#define TEST_BUILD
#include "../formant.cpp"
#include "../test_framework.h"
#include <cmath>
#include <numeric>

// 1. kBurstTCh パラメータ確認
REGISTER_TEST(burst_tch_params) {
    ASSERT_NEAR(kBurstTCh.f2, 3800.0, 0.01);
    ASSERT_TRUE(kBurstTCh.source == SourceType::Noise);
    ASSERT_NEAR(kBurstTCh.gain, 0.35, 0.01);
}

// 2. kBurstTs パラメータ確認
REGISTER_TEST(burst_ts_params) {
    ASSERT_NEAR(kBurstTs.f2, 5500.0, 0.01);
    ASSERT_TRUE(kBurstTs.source == SourceType::Noise);
    ASSERT_NEAR(kBurstTs.gain, 0.35, 0.01);
}

// 3. ち [tɕi] — makePlosiveCV が4エントリ返すか (closure + burst + frication + vowel)
REGISTER_TEST(chi_affricate_entry_count) {
    auto entries = makePlosiveCV(70, 5, 80, kSilence, kBurstTCh, kFricSh, kVowelI, 120);
    ASSERT_EQ(static_cast<int>(entries.size()), 4);
}

// 4. つ [tsɯ] — makePlosiveCV が4エントリ返すか
REGISTER_TEST(tsu_affricate_entry_count) {
    auto entries = makePlosiveCV(70, 5, 70, kSilence, kBurstTs, kFricS, kVowelU, 120);
    ASSERT_EQ(static_cast<int>(entries.size()), 4);
}

// 5. じ [dʑi] — makePlosiveCV が4エントリ返すか (voice bar + burst + frication + vowel)
REGISTER_TEST(ji_voiced_affricate_entry_count) {
    auto entries = makePlosiveCV(40, 5, 60, kVoiceBar, kBurstTCh, kFricZh, kVowelI, 120);
    ASSERT_EQ(static_cast<int>(entries.size()), 4);
}

// 6. ず [dzɯ] — makePlosiveCV が4エントリ返すか
REGISTER_TEST(zu_voiced_affricate_entry_count) {
    auto entries = makePlosiveCV(40, 5, 50, kVoiceBar, kBurstTs, kFricZ, kVowelU, 120);
    ASSERT_EQ(static_cast<int>(entries.size()), 4);
}

// 7. ち音節を合成し出力サイズが正しく、非ゼロ出力があるか
REGISTER_TEST(chi_synthesize_output) {
    auto entries = makePlosiveCV(70, 5, 80, kSilence, kBurstTCh, kFricSh, kVowelI, 120);
    int expected_samples = 0;
    for (auto& e : entries) expected_samples += e.duration_samples;

    Synthesizer synth;
    std::vector<int16_t> output;
    synth.synthesize(entries, output);

    ASSERT_EQ(static_cast<int>(output.size()), expected_samples);

    bool has_nonzero = false;
    for (auto s : output) {
        if (s != 0) { has_nonzero = true; break; }
    }
    ASSERT_TRUE(has_nonzero);
}

// 8. つ音節を合成し出力サイズが正しく、非ゼロ出力があるか
REGISTER_TEST(tsu_synthesize_output) {
    auto entries = makePlosiveCV(70, 5, 70, kSilence, kBurstTs, kFricS, kVowelU, 120);
    int expected_samples = 0;
    for (auto& e : entries) expected_samples += e.duration_samples;

    Synthesizer synth;
    std::vector<int16_t> output;
    synth.synthesize(entries, output);

    ASSERT_EQ(static_cast<int>(output.size()), expected_samples);

    bool has_nonzero = false;
    for (auto s : output) {
        if (s != 0) { has_nonzero = true; break; }
    }
    ASSERT_TRUE(has_nonzero);
}

// 9. じ音節を合成し出力サイズが正しく、非ゼロ出力があるか
REGISTER_TEST(ji_synthesize_output) {
    auto entries = makePlosiveCV(40, 5, 60, kVoiceBar, kBurstTCh, kFricZh, kVowelI, 120);
    int expected_samples = 0;
    for (auto& e : entries) expected_samples += e.duration_samples;

    Synthesizer synth;
    std::vector<int16_t> output;
    synth.synthesize(entries, output);

    ASSERT_EQ(static_cast<int>(output.size()), expected_samples);

    bool has_nonzero = false;
    for (auto s : output) {
        if (s != 0) { has_nonzero = true; break; }
    }
    ASSERT_TRUE(has_nonzero);
}

// 10. 破擦音のfrication区間(80ms)が破裂音のVOT(30ms)より長いことを構造的に確認
REGISTER_TEST(affricate_frication_longer_than_plosive_vot) {
    // 破擦音 ち: frication = 80ms
    auto affricate = makePlosiveCV(70, 5, 80, kSilence, kBurstTCh, kFricSh, kVowelI, 120);
    // 破裂音 か: VOT = 30ms
    auto plosive = makePlosiveCV(80, 10, 30, kSilence, kBurstK_A, kVotK_A, kVowelA, 120);

    // 両方とも4エントリ: [0]=closure, [1]=burst, [2]=frication/VOT, [3]=vowel
    ASSERT_EQ(static_cast<int>(affricate.size()), 4);
    ASSERT_EQ(static_cast<int>(plosive.size()), 4);

    int affricate_fric_samples = affricate[2].duration_samples;  // 80ms
    int plosive_vot_samples = plosive[2].duration_samples;       // 30ms

    ASSERT_GT(affricate_fric_samples, plosive_vot_samples);
}

int main() {
    return run_all_tests("Affricates");
}
