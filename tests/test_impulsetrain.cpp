#define TEST_BUILD
#include "../formant.cpp"
#include "../test_framework.h"

// Test 1: f0=100Hz, fs=44100Hz — first pulse appears near sample 441
REGISTER_TEST(first_pulse_at_correct_position) {
    ImpulseTrain it;
    constexpr double f0 = 100.0;
    constexpr double fs = 44100.0;
    int first_pulse = -1;

    for (int i = 0; i < 500; ++i) {
        double val = it.next(f0, fs);
        if (val == 1.0 && first_pulse < 0) {
            first_pulse = i;
        }
    }

    // period = fs/f0 = 441 samples, first pulse at sample 440 (0-indexed, when phase wraps)
    ASSERT_TRUE(first_pulse >= 0);
    ASSERT_NEAR(static_cast<double>(first_pulse), 441.0, 2.0);
}

// Test 2: non-pulse samples are all 0.0
REGISTER_TEST(non_pulse_samples_are_zero) {
    ImpulseTrain it;
    constexpr double f0 = 100.0;
    constexpr double fs = 44100.0;
    int pulse_count = 0;
    int zero_count = 0;

    for (int i = 0; i < 882; ++i) {
        double val = it.next(f0, fs);
        if (val == 1.0) {
            ++pulse_count;
        } else {
            ASSERT_EQ(val, 0.0);
            ++zero_count;
        }
    }

    // In 882 samples (2 periods), expect exactly 2 pulses
    ASSERT_EQ(pulse_count, 2);
    ASSERT_EQ(zero_count, 880);
}

// Test 3: second pulse appears ~441 samples after the first
REGISTER_TEST(consistent_period_between_pulses) {
    ImpulseTrain it;
    constexpr double f0 = 100.0;
    constexpr double fs = 44100.0;
    int first_pulse = -1;
    int second_pulse = -1;

    for (int i = 0; i < 1000; ++i) {
        double val = it.next(f0, fs);
        if (val == 1.0) {
            if (first_pulse < 0) {
                first_pulse = i;
            } else if (second_pulse < 0) {
                second_pulse = i;
                break;
            }
        }
    }

    ASSERT_TRUE(first_pulse >= 0);
    ASSERT_TRUE(second_pulse >= 0);
    int period = second_pulse - first_pulse;
    ASSERT_NEAR(static_cast<double>(period), 441.0, 2.0);
}

// Test 4: f0=200Hz — pulse period ~220.5 samples
REGISTER_TEST(period_correct_for_200hz) {
    ImpulseTrain it;
    constexpr double f0 = 200.0;
    constexpr double fs = 44100.0;
    int first_pulse = -1;
    int second_pulse = -1;

    for (int i = 0; i < 500; ++i) {
        double val = it.next(f0, fs);
        if (val == 1.0) {
            if (first_pulse < 0) {
                first_pulse = i;
            } else if (second_pulse < 0) {
                second_pulse = i;
                break;
            }
        }
    }

    ASSERT_TRUE(first_pulse >= 0);
    ASSERT_TRUE(second_pulse >= 0);
    int period = second_pulse - first_pulse;
    // Expected period: 44100/200 = 220.5, so integer period is 220 or 221
    ASSERT_NEAR(static_cast<double>(period), 220.5, 1.5);
}

// Test 5: extreme case f0=fs — no crash, produces pulses every sample
REGISTER_TEST(extreme_f0_equals_fs_no_crash) {
    ImpulseTrain it;
    constexpr double fs = 44100.0;
    constexpr double f0 = fs;
    int pulse_count = 0;

    for (int i = 0; i < 100; ++i) {
        double val = it.next(f0, fs);
        if (val == 1.0) {
            ++pulse_count;
        }
    }

    // f0/fs = 1.0, so phase increments by 1.0 each sample — every sample is a pulse
    ASSERT_EQ(pulse_count, 100);
}

int main() {
    return run_all_tests("ImpulseTrain");
}
