// test_synth_basic.cpp — Unit tests for Synthesizer class basic behavior
#define TEST_BUILD
#include "../formant.cpp"
#include "../test_framework.h"

// 1. 空のシーケンスで出力サイズが0か
REGISTER_TEST(empty_sequence_produces_no_output) {
    Synthesizer synth;
    std::vector<PhonemeEntry> sequence;
    std::vector<int16_t> output;
    synth.synthesize(sequence, output);
    ASSERT_EQ(static_cast<int>(output.size()), 0);
}

// 2. 単一母音(kVowelA, 4410サンプル=100ms)で出力サイズが4410か
REGISTER_TEST(single_vowel_output_size) {
    Synthesizer synth;
    std::vector<PhonemeEntry> sequence = {{kVowelA, 4410}};
    std::vector<int16_t> output;
    synth.synthesize(sequence, output);
    ASSERT_EQ(static_cast<int>(output.size()), 4410);
}

// 3. 複数エントリ(kVowelA 2205 + kVowelI 2205)で出力サイズが4410か
REGISTER_TEST(multiple_entries_output_size) {
    Synthesizer synth;
    std::vector<PhonemeEntry> sequence = {
        {kVowelA, 2205},
        {kVowelI, 2205},
    };
    std::vector<int16_t> output;
    synth.synthesize(sequence, output);
    ASSERT_EQ(static_cast<int>(output.size()), 4410);
}

// 4. 出力値が全て[-32768, 32767]の範囲内か
REGISTER_TEST(output_within_int16_range) {
    Synthesizer synth;
    std::vector<PhonemeEntry> sequence = {{kVowelA, 5000}};
    std::vector<int16_t> output;
    synth.synthesize(sequence, output);

    for (int i = 0; i < static_cast<int>(output.size()); ++i) {
        ASSERT_GE(static_cast<int>(output[i]), -32768);
        ASSERT_LE(static_cast<int>(output[i]), 32767);
    }
}

// 5. kSilence(gain=0)のみのシーケンスで出力の絶対値の最大が100未満か
REGISTER_TEST(silence_produces_near_zero_output) {
    Synthesizer synth;
    std::vector<PhonemeEntry> sequence = {{kSilence, 1000}};
    std::vector<int16_t> output;
    synth.synthesize(sequence, output);

    int16_t max_abs = 0;
    for (auto s : output) {
        int16_t a = static_cast<int16_t>(std::abs(static_cast<int>(s)));
        if (a > max_abs) max_abs = a;
    }
    ASSERT_LT(static_cast<int>(max_abs), 100);
}

// 6. gain=1.0の母音のRMSがgain=0.3の子音のRMSより大きいか
REGISTER_TEST(higher_gain_produces_larger_rms) {
    auto compute_rms = [](const FormantParams& params, int samples) {
        Synthesizer synth;
        std::vector<PhonemeEntry> sequence = {{params, samples}};
        std::vector<int16_t> output;
        synth.synthesize(sequence, output);

        double sum_sq = 0.0;
        for (auto s : output) {
            sum_sq += static_cast<double>(s) * static_cast<double>(s);
        }
        return std::sqrt(sum_sq / output.size());
    };

    int n = 4410; // 100ms
    double rms_vowel = compute_rms(kVowelA, n);      // gain=1.0
    double rms_nasal = compute_rms(kNasalN, n);       // gain=0.3

    ASSERT_GT(rms_vowel, rms_nasal);
}

int main() {
    return run_all_tests("SynthBasic");
}
