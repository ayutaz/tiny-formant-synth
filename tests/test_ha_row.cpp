// test_ha_row.cpp — は行条件異音テスト
#define TEST_BUILD
#include "../formant.cpp"
#include "../test_framework.h"
#include <cmath>
#include <numeric>

// 合成ヘルパー: 摩擦音 + 母音のCV音節を合成して出力を返す
static std::vector<int16_t> synth_cv(const FormantParams& fric, int fric_dur,
                                      const FormantParams& vowel, int vowel_dur) {
    Synthesizer synth;
    std::vector<PhonemeEntry> seq = {{fric, fric_dur}, {vowel, vowel_dur}};
    std::vector<int16_t> out;
    synth.synthesize(seq, out);
    return out;
}

// 1. は [ha] = kFricH_A(80ms) + kVowelA
REGISTER_TEST(ha_synthesize) {
    int fric_dur = ms2s(80);
    int vowel_dur = ms2s(120);
    auto out = synth_cv(kFricH_A, fric_dur, kVowelA, vowel_dur);
    ASSERT_EQ(static_cast<int>(out.size()), fric_dur + vowel_dur);
    // 非ゼロサンプルが存在すること
    bool has_nonzero = false;
    for (auto s : out) { if (s != 0) { has_nonzero = true; break; } }
    ASSERT_TRUE(has_nonzero);
}

// 2. ひ [çi] = kFricChi(100ms) + kVowelI
REGISTER_TEST(hi_synthesize) {
    int fric_dur = ms2s(100);
    int vowel_dur = ms2s(120);
    auto out = synth_cv(kFricChi, fric_dur, kVowelI, vowel_dur);
    ASSERT_EQ(static_cast<int>(out.size()), fric_dur + vowel_dur);
    bool has_nonzero = false;
    for (auto s : out) { if (s != 0) { has_nonzero = true; break; } }
    ASSERT_TRUE(has_nonzero);
}

// 3. ふ [ɸɯ] = kFricPhi(100ms) + kVowelU
REGISTER_TEST(fu_synthesize) {
    int fric_dur = ms2s(100);
    int vowel_dur = ms2s(120);
    auto out = synth_cv(kFricPhi, fric_dur, kVowelU, vowel_dur);
    ASSERT_EQ(static_cast<int>(out.size()), fric_dur + vowel_dur);
    bool has_nonzero = false;
    for (auto s : out) { if (s != 0) { has_nonzero = true; break; } }
    ASSERT_TRUE(has_nonzero);
}

// 4. へ [he] = kFricH_E(80ms) + kVowelE
REGISTER_TEST(he_synthesize) {
    int fric_dur = ms2s(80);
    int vowel_dur = ms2s(120);
    auto out = synth_cv(kFricH_E, fric_dur, kVowelE, vowel_dur);
    ASSERT_EQ(static_cast<int>(out.size()), fric_dur + vowel_dur);
    bool has_nonzero = false;
    for (auto s : out) { if (s != 0) { has_nonzero = true; break; } }
    ASSERT_TRUE(has_nonzero);
}

// 5. ほ [ho] = kFricH_O(80ms) + kVowelO
REGISTER_TEST(ho_synthesize) {
    int fric_dur = ms2s(80);
    int vowel_dur = ms2s(120);
    auto out = synth_cv(kFricH_O, fric_dur, kVowelO, vowel_dur);
    ASSERT_EQ(static_cast<int>(out.size()), fric_dur + vowel_dur);
    bool has_nonzero = false;
    for (auto s : out) { if (s != 0) { has_nonzero = true; break; } }
    ASSERT_TRUE(has_nonzero);
}

// 6. ひ は異なる摩擦音を使う: kFricChi.f2 != kFricH_A.f2
REGISTER_TEST(hi_uses_different_fricative_than_ha) {
    ASSERT_TRUE(kFricChi.f2 != kFricH_A.f2);
}

// 7. ふ は異なる摩擦音を使う: kFricPhi.f2 != kFricH_A.f2
REGISTER_TEST(fu_uses_different_fricative_than_ha) {
    ASSERT_TRUE(kFricPhi.f2 != kFricH_A.f2);
}

// 8. は行の5つの摩擦音パラメータ全てが source == Noise
REGISTER_TEST(all_ha_row_source_is_noise) {
    ASSERT_TRUE(kFricH_A.source == SourceType::Noise);
    ASSERT_TRUE(kFricChi.source == SourceType::Noise);
    ASSERT_TRUE(kFricPhi.source == SourceType::Noise);
    ASSERT_TRUE(kFricH_E.source == SourceType::Noise);
    ASSERT_TRUE(kFricH_O.source == SourceType::Noise);
}

int main() {
    return run_all_tests("HaRow");
}
