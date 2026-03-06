// test_text_to_phoneme.cpp — Unit tests for textToPhoneme (UTF-8 decode + kana mapping)
#define TEST_BUILD
#include "../formant.cpp"
#include "../test_framework.h"

// 1. Single vowel "a"
REGISTER_TEST(vowel_a) {
    auto seq = textToPhoneme("あ");
    ASSERT_EQ(static_cast<int>(seq.size()), 1);
    ASSERT_NEAR(seq[0].params.f1, kVowelA.f1, 0.01);
    ASSERT_NEAR(seq[0].params.f2, kVowelA.f2, 0.01);
}

// 2. All five vowels
REGISTER_TEST(five_vowels) {
    auto seq = textToPhoneme("あいうえお");
    ASSERT_EQ(static_cast<int>(seq.size()), 5);
    ASSERT_NEAR(seq[0].params.f1, kVowelA.f1, 0.01);
    ASSERT_NEAR(seq[1].params.f1, kVowelI.f1, 0.01);
    ASSERT_NEAR(seq[2].params.f1, kVowelU.f1, 0.01);
    ASSERT_NEAR(seq[3].params.f1, kVowelE.f1, 0.01);
    ASSERT_NEAR(seq[4].params.f1, kVowelO.f1, 0.01);
}

// 3. ka-row plosive: "ka" produces 4 entries (closure+burst+vot+vowel)
REGISTER_TEST(ka_plosive) {
    auto seq = textToPhoneme("か");
    ASSERT_EQ(static_cast<int>(seq.size()), 4);
    // Last entry is the vowel
    ASSERT_NEAR(seq[3].params.f1, kVowelA.f1, 0.01);
}

// 4. sa-row fricative: "sa" produces 2 entries (fricative+vowel)
REGISTER_TEST(sa_fricative) {
    auto seq = textToPhoneme("さ");
    ASSERT_EQ(static_cast<int>(seq.size()), 2);
    ASSERT_NEAR(seq[0].params.f2, kFricS.f2, 0.01);
}

// 5. "shi" allophone: uses kFricSh (f2=3800), not kFricS (f2=5500)
REGISTER_TEST(shi_allophone) {
    auto seq = textToPhoneme("し");
    ASSERT_EQ(static_cast<int>(seq.size()), 2);
    ASSERT_NEAR(seq[0].params.f2, kFricSh.f2, 0.01);
}

// 6. ta-row plosive: "ta" produces 4 entries
REGISTER_TEST(ta_plosive) {
    auto seq = textToPhoneme("た");
    ASSERT_EQ(static_cast<int>(seq.size()), 4);
    // Burst should be kBurstT
    ASSERT_NEAR(seq[1].params.f2, kBurstT.f2, 0.01);
}

// 7. "chi" allophone: affricate, frication part uses kFricSh
REGISTER_TEST(chi_affricate) {
    auto seq = textToPhoneme("ち");
    ASSERT_EQ(static_cast<int>(seq.size()), 4);
    // Entry 2 is frication (VOT slot uses kFricSh for CH)
    ASSERT_NEAR(seq[2].params.f2, kFricSh.f2, 0.01);
}

// 8. "tsu" allophone: affricate, frication part uses kFricS
REGISTER_TEST(tsu_affricate) {
    auto seq = textToPhoneme("つ");
    ASSERT_EQ(static_cast<int>(seq.size()), 4);
    // Entry 2 is frication (VOT slot uses kFricS for TS)
    ASSERT_NEAR(seq[2].params.f2, kFricS.f2, 0.01);
}

// 9. ha-row allophones: ha->kFricH_A, hi->kFricChi, hu->kFricPhi
REGISTER_TEST(ha_row_allophones) {
    auto ha = textToPhoneme("は");
    ASSERT_EQ(static_cast<int>(ha.size()), 2);
    ASSERT_NEAR(ha[0].params.f2, kFricH_A.f2, 0.01);

    auto hi = textToPhoneme("ひ");
    ASSERT_EQ(static_cast<int>(hi.size()), 2);
    ASSERT_NEAR(hi[0].params.f2, kFricChi.f2, 0.01);

    auto hu = textToPhoneme("ふ");
    ASSERT_EQ(static_cast<int>(hu.size()), 2);
    ASSERT_NEAR(hu[0].params.f2, kFricPhi.f2, 0.01);
}

// 10. Voiced plosive "ga": 3 entries (closure+burst+vowel, vot=0 so no VOT entry)
REGISTER_TEST(ga_voiced_plosive) {
    auto seq = textToPhoneme("が");
    ASSERT_EQ(static_cast<int>(seq.size()), 3);
    // First entry (closure) uses voice bar (Impulse source)
    ASSERT_TRUE(seq[0].params.source == SourceType::Impulse);
    ASSERT_NEAR(seq[0].params.f1, kVoiceBar.f1, 0.01);
}

// 11. Yotsugana: "di" and "ji" produce same number of entries with same source type
REGISTER_TEST(yotsugana_di_ji) {
    auto di = textToPhoneme("ぢ");
    auto ji = textToPhoneme("じ");
    ASSERT_EQ(static_cast<int>(di.size()), static_cast<int>(ji.size()));
    // Both should have same source type on frication segment
    for (std::size_t i = 0; i < di.size(); ++i) {
        ASSERT_TRUE(di[i].params.source == ji[i].params.source);
    }
}

// 12. "wo" maps same as "o"
REGISTER_TEST(wo_equals_o) {
    auto wo = textToPhoneme("を");
    auto o  = textToPhoneme("お");
    ASSERT_EQ(static_cast<int>(wo.size()), 1);
    ASSERT_NEAR(wo[0].params.f1, o[0].params.f1, 0.01);
}

// 13. Empty string
REGISTER_TEST(empty_string) {
    auto seq = textToPhoneme("");
    ASSERT_EQ(static_cast<int>(seq.size()), 0);
}

// 14. Non-hiragana ignored
REGISTER_TEST(non_hiragana_ignored) {
    auto seq = textToPhoneme("abc");
    ASSERT_EQ(static_cast<int>(seq.size()), 0);
}

int main() {
    return run_all_tests("TextToPhoneme");
}
