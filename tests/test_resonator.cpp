// test_resonator.cpp — Unit tests for Resonator struct
#define TEST_BUILD
#include "../formant.cpp"
#include "../test_framework.h"

// 1. set() で係数が適切な値になるか
REGISTER_TEST(set_coefficients_valid) {
    Resonator r;
    r.set(1000.0, 100.0, kSampleRate);

    // NaN/Inf でないこと
    ASSERT_FALSE(std::isnan(r.a0));
    ASSERT_FALSE(std::isnan(r.b1));
    ASSERT_FALSE(std::isnan(r.b2));
    ASSERT_FALSE(std::isinf(r.a0));
    ASSERT_FALSE(std::isinf(r.b1));
    ASSERT_FALSE(std::isinf(r.b2));

    // b2 = R^2 は 0 < b2 < 1 の範囲
    ASSERT_GT(r.b2, 0.0);
    ASSERT_LT(r.b2, 1.0);
}

// 2. reset() で z1, z2 が 0 になるか
REGISTER_TEST(reset_clears_state) {
    Resonator r;
    r.set(1000.0, 100.0, kSampleRate);
    // 何かしら状態を持たせる
    r.process(1.0);
    r.process(0.0);
    ASSERT_TRUE(r.z1 != 0.0 || r.z2 != 0.0);

    r.reset();
    ASSERT_EQ(r.z1, 0.0);
    ASSERT_EQ(r.z2, 0.0);
}

// 3. インパルス応答が減衰するか（安定性）
REGISTER_TEST(impulse_response_decays) {
    Resonator r;
    r.set(1000.0, 100.0, kSampleRate);

    // インパルス入力
    double first = std::abs(r.process(1.0));
    // 後続 500 サンプル (以降は入力 0)
    double peak = first;
    double last = 0.0;
    for (int i = 0; i < 500; ++i) {
        double y = std::abs(r.process(0.0));
        if (y > peak) peak = y;
        last = y;
    }

    // 500 サンプル後には十分減衰しているはず
    ASSERT_GT(peak, 0.0);
    ASSERT_LT(last, peak * 0.1);
}

// 4. 共振周波数付近のサイン波は振幅が大きく、遠い周波数では小さい
REGISTER_TEST(resonance_frequency_response) {
    auto measure_rms = [](double freq, double center_freq, double bw) {
        Resonator r;
        r.set(center_freq, bw, kSampleRate);

        double sum_sq = 0.0;
        int N = 4410; // 100ms 分
        for (int i = 0; i < N; ++i) {
            double input = std::sin(2.0 * kPi * freq * i / kSampleRate);
            double y = r.process(input);
            // 最初の 200 サンプルは過渡応答なのでスキップ
            if (i >= 200) {
                sum_sq += y * y;
            }
        }
        return std::sqrt(sum_sq / (N - 200));
    };

    double center = 1000.0;
    double bw = 100.0;

    double rms_on  = measure_rms(center, center, bw);        // 共振周波数
    double rms_off = measure_rms(5000.0, center, bw);        // 遠い周波数

    // 共振周波数の出力は遠い周波数よりはるかに大きい
    ASSERT_GT(rms_on, rms_off * 3.0);
}

// 5. 帯域幅が広いほうが減衰が速い
REGISTER_TEST(wider_bandwidth_decays_faster) {
    auto measure_decay = [](double bw) {
        Resonator r;
        r.set(1000.0, bw, kSampleRate);

        r.process(1.0);
        double energy = 0.0;
        // 200 サンプル分のエネルギーを測定
        for (int i = 0; i < 200; ++i) {
            double y = r.process(0.0);
            energy += y * y;
        }
        return energy;
    };

    double energy_narrow = measure_decay(50.0);   // 狭い帯域幅
    double energy_wide   = measure_decay(500.0);  // 広い帯域幅

    // 狭い帯域幅のほうが長く振動 → エネルギーが大きい
    ASSERT_GT(energy_narrow, energy_wide);
}

int main() {
    return run_all_tests("Resonator");
}
