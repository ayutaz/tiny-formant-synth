// test_synth_source.cpp — Tests for Synthesizer source switching & formant transition
#define TEST_BUILD
#include "../formant.cpp"
#include "../test_framework.h"
#include <numeric>

// RMS計算ヘルパー
static double calc_rms(const std::vector<int16_t>& buf, int start = 0, int end = -1) {
    if (end < 0) end = static_cast<int>(buf.size());
    double sum_sq = 0.0;
    int count = end - start;
    for (int i = start; i < end; ++i) {
        double v = static_cast<double>(buf[i]);
        sum_sq += v * v;
    }
    return std::sqrt(sum_sq / count);
}

// 非ゼロサンプル数カウント
static int count_nonzero(const std::vector<int16_t>& buf) {
    int count = 0;
    for (auto s : buf) {
        if (s != 0) ++count;
    }
    return count;
}

// 1. Impulse音源: kVowelA(4410サンプル)を合成し、出力に非ゼロ値が多数あるか
REGISTER_TEST(impulse_source_produces_output) {
    Synthesizer synth;
    std::vector<int16_t> buf;
    std::vector<PhonemeEntry> seq = {{kVowelA, 4410}};
    synth.synthesize(seq, buf);

    ASSERT_EQ(static_cast<int>(buf.size()), 4410);
    int nz = count_nonzero(buf);
    // インパルス音源の母音: 多数の非ゼロサンプルが期待される
    ASSERT_GT(nz, 100);
}

// 2. Noise音源: kFricS(4410サンプル)を合成し、出力に非ゼロ値が多数あるか
REGISTER_TEST(noise_source_produces_output) {
    Synthesizer synth;
    std::vector<int16_t> buf;
    std::vector<PhonemeEntry> seq = {{kFricS, 4410}};
    synth.synthesize(seq, buf);

    ASSERT_EQ(static_cast<int>(buf.size()), 4410);
    int nz = count_nonzero(buf);
    // ノイズ音源の摩擦音: ほぼ全サンプルが非ゼロ
    ASSERT_GT(nz, 1000);
}

// 3. 母音→母音の遷移: kVowelA(4410) → kVowelI(4410) を合成し、
//    境界付近で急激な不連続がないか
REGISTER_TEST(vowel_transition_no_discontinuity) {
    Synthesizer synth;
    std::vector<int16_t> buf;
    std::vector<PhonemeEntry> seq = {{kVowelA, 4410}, {kVowelI, 4410}};
    synth.synthesize(seq, buf);

    ASSERT_EQ(static_cast<int>(buf.size()), 8820);

    // 境界付近 (4400..4420) の隣接サンプル差を検査
    // 急激な不連続がないことを確認
    int boundary_start = 4400;
    int boundary_end = 4420;
    double max_diff = 0.0;
    for (int i = boundary_start; i < boundary_end && i + 1 < static_cast<int>(buf.size()); ++i) {
        double diff = std::abs(static_cast<double>(buf[i + 1]) - static_cast<double>(buf[i]));
        if (diff > max_diff) max_diff = diff;
    }
    // 隣接サンプル差が全レンジ(65536)の50%未満であること
    ASSERT_LT(max_diff, 32768.0);
}

// 4. gainが異なるセグメント: kVowelA(gain=1.0) と kNasalN(gain=0.3) の
//    RMSを比較してkVowelAの方が大きいか
REGISTER_TEST(gain_affects_rms) {
    // kVowelA (gain=1.0) を合成
    Synthesizer synth1;
    std::vector<int16_t> buf1;
    std::vector<PhonemeEntry> seq1 = {{kVowelA, 4410}};
    synth1.synthesize(seq1, buf1);

    // kNasalN (gain=0.3) を合成
    Synthesizer synth2;
    std::vector<int16_t> buf2;
    std::vector<PhonemeEntry> seq2 = {{kNasalN, 4410}};
    synth2.synthesize(seq2, buf2);

    double rms_vowel = calc_rms(buf1);
    double rms_nasal = calc_rms(buf2);

    // 両方とも非ゼロ出力がある
    ASSERT_GT(rms_vowel, 0.0);
    ASSERT_GT(rms_nasal, 0.0);
    // gain=1.0 の母音の方がgain=0.3 の鼻音よりRMSが大きい
    ASSERT_GT(rms_vowel, rms_nasal);
}

// 5. 異なるフォルマント: kVowelA と kVowelI のRMSが共に非ゼロで、
//    値が異なるか（異なるスペクトル特性）
REGISTER_TEST(different_formants_produce_different_rms) {
    Synthesizer synth1;
    std::vector<int16_t> buf1;
    std::vector<PhonemeEntry> seq1 = {{kVowelA, 4410}};
    synth1.synthesize(seq1, buf1);

    Synthesizer synth2;
    std::vector<int16_t> buf2;
    std::vector<PhonemeEntry> seq2 = {{kVowelI, 4410}};
    synth2.synthesize(seq2, buf2);

    double rms_a = calc_rms(buf1);
    double rms_i = calc_rms(buf2);

    // 両方とも非ゼロ
    ASSERT_GT(rms_a, 0.0);
    ASSERT_GT(rms_i, 0.0);
    // 異なるフォルマント設定 → 異なるRMS値
    double diff = std::abs(rms_a - rms_i);
    ASSERT_GT(diff, 0.0);
}

// 6. Mixed音源: kFricZ(4410サンプル)を合成し、非ゼロサンプルが半数以上か
REGISTER_TEST(mixed_source_produces_nonzero_output) {
    Synthesizer synth;
    std::vector<int16_t> buf;
    std::vector<PhonemeEntry> seq = {{kFricZ, 4410}};
    synth.synthesize(seq, buf);

    ASSERT_EQ(static_cast<int>(buf.size()), 4410);
    int nz = count_nonzero(buf);
    // Mixed音源: 半数以上が非ゼロ
    ASSERT_GT(nz, 2205);
}

// 7. Mixed音源: kFricZ(4410サンプル)合成のRMSが0より大きいか
REGISTER_TEST(mixed_source_rms_positive) {
    Synthesizer synth;
    std::vector<int16_t> buf;
    std::vector<PhonemeEntry> seq = {{kFricZ, 4410}};
    synth.synthesize(seq, buf);

    double rms = calc_rms(buf);
    ASSERT_GT(rms, 0.0);
}

// 8. Mixed音源とNoise音源の違い: kFricZ(Mixed)とkFricS(Noise)で合成した
//    結果のRMSが異なるか
REGISTER_TEST(mixed_differs_from_pure_noise) {
    Synthesizer synth1;
    std::vector<int16_t> buf1;
    std::vector<PhonemeEntry> seq1 = {{kFricZ, 4410}};
    synth1.synthesize(seq1, buf1);

    Synthesizer synth2;
    std::vector<int16_t> buf2;
    std::vector<PhonemeEntry> seq2 = {{kFricS, 4410}};
    synth2.synthesize(seq2, buf2);

    double rms_mixed = calc_rms(buf1);
    double rms_noise = calc_rms(buf2);

    // 両方とも非ゼロ
    ASSERT_GT(rms_mixed, 0.0);
    ASSERT_GT(rms_noise, 0.0);
    // Mixed と Noise は異なるRMS値
    double diff = std::abs(rms_mixed - rms_noise);
    ASSERT_GT(diff, 0.0);
}

int main() {
    return run_all_tests("SynthSource");
}
