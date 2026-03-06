// test_mixed_source.cpp — Tests for SourceType::Mixed (voiced fricatives)
#define TEST_BUILD
#include "../formant.cpp"
#include "../test_framework.h"
#include <numeric>
#include <cmath>

static double calc_rms(const std::vector<int16_t>& buf) {
    double sum_sq = 0.0;
    for (auto s : buf) {
        double v = static_cast<double>(s);
        sum_sq += v * v;
    }
    return std::sqrt(sum_sq / buf.size());
}

static int count_nonzero(const std::vector<int16_t>& buf) {
    int count = 0;
    for (auto s : buf) {
        if (s != 0) ++count;
    }
    return count;
}

// Helper: kFricZ with source overridden
static FormantParams withSource(FormantParams p, SourceType src) {
    p.source = src;
    return p;
}

// 1. Mixed音源(kFricZ)が非ゼロ出力を多数生成するか
REGISTER_TEST(mixed_source_produces_output) {
    Synthesizer synth;
    std::vector<int16_t> buf;
    std::vector<PhonemeEntry> seq = {{kFricZ, 4410}};
    synth.synthesize(seq, buf);

    ASSERT_EQ(static_cast<int>(buf.size()), 4410);
    int nz = count_nonzero(buf);
    ASSERT_GT(nz, 100);
}

// 2. Mixed音源の出力がNoise単体と異なるか
REGISTER_TEST(mixed_source_differs_from_noise) {
    Synthesizer synth_mixed;
    std::vector<int16_t> buf_mixed;
    std::vector<PhonemeEntry> seq_mixed = {{kFricZ, 4410}};
    synth_mixed.synthesize(seq_mixed, buf_mixed);

    Synthesizer synth_noise;
    std::vector<int16_t> buf_noise;
    std::vector<PhonemeEntry> seq_noise = {{withSource(kFricZ, SourceType::Noise), 4410}};
    synth_noise.synthesize(seq_noise, buf_noise);

    ASSERT_EQ(static_cast<int>(buf_mixed.size()), 4410);
    ASSERT_EQ(static_cast<int>(buf_noise.size()), 4410);

    int diffs = 0;
    for (int i = 0; i < 4410; ++i) {
        if (buf_mixed[i] != buf_noise[i]) ++diffs;
    }
    ASSERT_GT(diffs, 0);
}

// 3. Mixed音源の出力がImpulse単体と異なるか
REGISTER_TEST(mixed_source_differs_from_impulse) {
    Synthesizer synth_mixed;
    std::vector<int16_t> buf_mixed;
    std::vector<PhonemeEntry> seq_mixed = {{kFricZ, 4410}};
    synth_mixed.synthesize(seq_mixed, buf_mixed);

    Synthesizer synth_imp;
    std::vector<int16_t> buf_imp;
    std::vector<PhonemeEntry> seq_imp = {{withSource(kFricZ, SourceType::Impulse), 4410}};
    synth_imp.synthesize(seq_imp, buf_imp);

    ASSERT_EQ(static_cast<int>(buf_mixed.size()), 4410);
    ASSERT_EQ(static_cast<int>(buf_imp.size()), 4410);

    int diffs = 0;
    for (int i = 0; i < 4410; ++i) {
        if (buf_mixed[i] != buf_imp[i]) ++diffs;
    }
    ASSERT_GT(diffs, 0);
}

// 4. Mixed音源のRMSがNoise/Impulse単体と異なる（非ゼロ）
REGISTER_TEST(mixed_rms_between_noise_and_impulse) {
    Synthesizer synth_mixed;
    std::vector<int16_t> buf_mixed;
    synth_mixed.synthesize({{kFricZ, 4410}}, buf_mixed);

    Synthesizer synth_noise;
    std::vector<int16_t> buf_noise;
    synth_noise.synthesize({{withSource(kFricZ, SourceType::Noise), 4410}}, buf_noise);

    Synthesizer synth_imp;
    std::vector<int16_t> buf_imp;
    synth_imp.synthesize({{withSource(kFricZ, SourceType::Impulse), 4410}}, buf_imp);

    double rms_mixed = calc_rms(buf_mixed);
    double rms_noise = calc_rms(buf_noise);
    double rms_imp   = calc_rms(buf_imp);

    ASSERT_GT(rms_mixed, 0.0);
    ASSERT_GT(rms_noise, 0.0);
    ASSERT_GT(rms_imp, 0.0);

    // Mixed は Noise/Impulse どちらとも異なるRMSを持つ
    double diff_from_noise = std::abs(rms_mixed - rms_noise);
    double diff_from_imp   = std::abs(rms_mixed - rms_imp);
    ASSERT_GT(diff_from_noise + diff_from_imp, 0.0);
}

// 5. 有声摩擦音 /z/ → 母音 /a/ のCV音節が正しく合成されるか
REGISTER_TEST(voiced_fricative_z_has_output) {
    Synthesizer synth;
    std::vector<int16_t> buf;
    // kFricZ(摩擦区間) → kVowelA(母音)
    std::vector<PhonemeEntry> seq = {{kFricZ, 2205}, {kVowelA, 4410}};
    synth.synthesize(seq, buf);

    ASSERT_EQ(static_cast<int>(buf.size()), 2205 + 4410);
    int nz = count_nonzero(buf);
    ASSERT_GT(nz, 500);
}

// 6. 有声摩擦音 /zh/ → 母音 /i/ のCV音節が正しく合成されるか
REGISTER_TEST(voiced_fricative_zh_has_output) {
    Synthesizer synth;
    std::vector<int16_t> buf;
    // kFricZh(摩擦区間) → kVowelI(母音)
    std::vector<PhonemeEntry> seq = {{kFricZh, 2205}, {kVowelI, 4410}};
    synth.synthesize(seq, buf);

    ASSERT_EQ(static_cast<int>(buf.size()), 2205 + 4410);
    int nz = count_nonzero(buf);
    ASSERT_GT(nz, 500);
}

int main() {
    return run_all_tests("MixedSource");
}
