#define TEST_BUILD
#include "../formant.cpp"
#include "../test_framework.h"

// ============ ms2s tests ============

REGISTER_TEST(ms2s_1000ms_equals_44100) {
    ASSERT_EQ(ms2s(1000), 44100);
}

REGISTER_TEST(ms2s_0ms_equals_0) {
    ASSERT_EQ(ms2s(0), 0);
}

REGISTER_TEST(ms2s_120ms_equals_5292) {
    ASSERT_EQ(ms2s(120), 5292);
}

REGISTER_TEST(ms2s_80ms_equals_3528) {
    ASSERT_EQ(ms2s(80), 3528);
}

// ============ append tests ============

REGISTER_TEST(append_to_empty_sequence) {
    std::vector<PhonemeEntry> seq;
    std::vector<PhonemeEntry> entries = {
        {kVowelA, ms2s(100)},
        {kVowelI, ms2s(100)},
        {kVowelU, ms2s(100)},
    };
    append(seq, entries);
    ASSERT_EQ(static_cast<int>(seq.size()), 3);
}

REGISTER_TEST(append_to_existing_sequence) {
    std::vector<PhonemeEntry> seq = {
        {kVowelA, ms2s(100)},
        {kVowelI, ms2s(100)},
    };
    std::vector<PhonemeEntry> entries = {
        {kVowelU, ms2s(100)},
        {kVowelE, ms2s(100)},
        {kVowelO, ms2s(100)},
    };
    append(seq, entries);
    ASSERT_EQ(static_cast<int>(seq.size()), 5);
}

// ============ makePlosiveCV tests ============

REGISTER_TEST(makePlosiveCV_voiceless_returns_4_entries) {
    auto result = makePlosiveCV(80, 10, 30, kSilence, kBurstK_A, kVotK_A, kVowelA, 120);
    ASSERT_EQ(static_cast<int>(result.size()), 4);
}

REGISTER_TEST(makePlosiveCV_voiced_vot0_returns_3_entries) {
    auto result = makePlosiveCV(60, 5, 0, kVoiceBar, kBurstK_A, kVotK_A, kVowelA, 120);
    ASSERT_EQ(static_cast<int>(result.size()), 3);
}

REGISTER_TEST(makePlosiveCV_closure0_skips_closure) {
    auto result = makePlosiveCV(0, 10, 30, kSilence, kBurstT, kVotT_A, kVowelA, 120);
    ASSERT_EQ(static_cast<int>(result.size()), 3);
}

REGISTER_TEST(makePlosiveCV_duration_samples_match_ms2s) {
    auto result = makePlosiveCV(80, 10, 30, kSilence, kBurstK_A, kVotK_A, kVowelA, 120);
    ASSERT_EQ(result[0].duration_samples, ms2s(80));
    ASSERT_EQ(result[1].duration_samples, ms2s(10));
    ASSERT_EQ(result[2].duration_samples, ms2s(30));
    ASSERT_EQ(result[3].duration_samples, ms2s(120));
}

int main() {
    return run_all_tests("Helpers");
}
