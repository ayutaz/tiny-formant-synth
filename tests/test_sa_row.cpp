// test_sa_row.cpp — Unit tests for sa-row syllables (sa/shi/su/se/so)
#define TEST_BUILD
#include "../formant.cpp"
#include "../test_framework.h"
#include <cmath>
#include <numeric>

// Helper: compute RMS of output
static double compute_rms(const std::vector<int16_t>& output) {
    double sum_sq = 0.0;
    for (auto s : output) {
        sum_sq += static_cast<double>(s) * static_cast<double>(s);
    }
    return std::sqrt(sum_sq / output.size());
}

// Helper: synthesize a fricative + vowel syllable
static std::vector<int16_t> synth_syllable(const FormantParams& fric, const FormantParams& vowel, int fric_ms = 120, int vowel_ms = 120) {
    Synthesizer synth;
    std::vector<PhonemeEntry> sequence = {
        {fric,  ms2s(fric_ms)},
        {vowel, ms2s(vowel_ms)},
    };
    std::vector<int16_t> output;
    synth.synthesize(sequence, output);
    return output;
}

// 1. sa_synthesize — output size = ms2s(120)+ms2s(120), non-zero output
REGISTER_TEST(sa_synthesize) {
    auto output = synth_syllable(kFricS, kVowelA);
    int expected = ms2s(120) + ms2s(120);
    ASSERT_EQ(static_cast<int>(output.size()), expected);

    bool has_nonzero = false;
    for (auto s : output) {
        if (s != 0) { has_nonzero = true; break; }
    }
    ASSERT_TRUE(has_nonzero);
}

// 2. shi_synthesize — shi uses kFricSh, output is correct size
REGISTER_TEST(shi_synthesize) {
    auto output = synth_syllable(kFricSh, kVowelI);
    int expected = ms2s(120) + ms2s(120);
    ASSERT_EQ(static_cast<int>(output.size()), expected);

    bool has_nonzero = false;
    for (auto s : output) {
        if (s != 0) { has_nonzero = true; break; }
    }
    ASSERT_TRUE(has_nonzero);
}

// 3. su_synthesize
REGISTER_TEST(su_synthesize) {
    auto output = synth_syllable(kFricS, kVowelU);
    int expected = ms2s(120) + ms2s(120);
    ASSERT_EQ(static_cast<int>(output.size()), expected);

    bool has_nonzero = false;
    for (auto s : output) {
        if (s != 0) { has_nonzero = true; break; }
    }
    ASSERT_TRUE(has_nonzero);
}

// 4. se_synthesize
REGISTER_TEST(se_synthesize) {
    auto output = synth_syllable(kFricS, kVowelE);
    int expected = ms2s(120) + ms2s(120);
    ASSERT_EQ(static_cast<int>(output.size()), expected);

    bool has_nonzero = false;
    for (auto s : output) {
        if (s != 0) { has_nonzero = true; break; }
    }
    ASSERT_TRUE(has_nonzero);
}

// 5. so_synthesize
REGISTER_TEST(so_synthesize) {
    auto output = synth_syllable(kFricS, kVowelO);
    int expected = ms2s(120) + ms2s(120);
    ASSERT_EQ(static_cast<int>(output.size()), expected);

    bool has_nonzero = false;
    for (auto s : output) {
        if (s != 0) { has_nonzero = true; break; }
    }
    ASSERT_TRUE(has_nonzero);
}

// 6. shi_uses_kFricSh_not_kFricS — shi uses kFricSh (F2=3800 != 5500)
REGISTER_TEST(shi_uses_kFricSh_not_kFricS) {
    ASSERT_NEAR(kFricSh.f2, 3800.0, 0.01);
    ASSERT_NEAR(kFricS.f2,  5500.0, 0.01);
    // Confirm they are distinct
    ASSERT_TRUE(std::abs(kFricSh.f2 - kFricS.f2) > 1000.0);
}

// 7. sa_row_all_source_noise — all sa-row fricative params use Noise source
REGISTER_TEST(sa_row_all_source_noise) {
    ASSERT_TRUE(kFricS.source  == SourceType::Noise);
    ASSERT_TRUE(kFricSh.source == SourceType::Noise);
}

// 8. shi_rms_nonzero — shi syllable RMS > 0
REGISTER_TEST(shi_rms_nonzero) {
    auto output = synth_syllable(kFricSh, kVowelI);
    double rms = compute_rms(output);
    ASSERT_GT(rms, 0.0);
}

int main() {
    return run_all_tests("SaRow");
}
