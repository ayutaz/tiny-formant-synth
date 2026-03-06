// test_youon_special.cpp -- Tests for youon, sokuon, hatsuon, chouon
#define TEST_BUILD
#include "../formant.cpp"
#include "../test_framework.h"

// --- Youon tests ---

// 1. きゃ: K plosive + vowel A (closure+burst+vot+vowel = 4 entries)
REGISTER_TEST(kya_youon) {
    auto seq = textToPhoneme("きゃ");
    ASSERT_EQ(static_cast<int>(seq.size()), 4);
    // Last entry should be vowel A
    ASSERT_NEAR(seq.back().params.f1, kVowelA.f1, 0.01);
    ASSERT_NEAR(seq.back().params.f2, kVowelA.f2, 0.01);
    ASSERT_EQ(seq.back().duration_samples, ms2s(120));
}

// 2. しゃ: SH fricative + vowel A (2 entries)
REGISTER_TEST(sha_youon) {
    auto seq = textToPhoneme("しゃ");
    ASSERT_EQ(static_cast<int>(seq.size()), 2);
    ASSERT_NEAR(seq[0].params.f2, kFricSh.f2, 0.01);  // 3800
    ASSERT_NEAR(seq[1].params.f1, kVowelA.f1, 0.01);   // 800
}

// 3. ちゃ: CH affricate + vowel A (closure+burst+frication+vowel = 4 entries)
REGISTER_TEST(cha_youon) {
    auto seq = textToPhoneme("ちゃ");
    ASSERT_EQ(static_cast<int>(seq.size()), 4);
    ASSERT_NEAR(seq.back().params.f1, kVowelA.f1, 0.01);
    ASSERT_NEAR(seq.back().params.f2, kVowelA.f2, 0.01);
}

// 4. ひゃ: HI fricative + vowel A (2 entries)
REGISTER_TEST(hya_youon) {
    auto seq = textToPhoneme("ひゃ");
    ASSERT_EQ(static_cast<int>(seq.size()), 2);
    ASSERT_NEAR(seq[0].params.f2, kFricChi.f2, 0.01);  // 3000
    ASSERT_NEAR(seq[1].params.f1, kVowelA.f1, 0.01);   // 800
}

// 5. きゅ: K plosive + vowel U
REGISTER_TEST(kyu_youon) {
    auto seq = textToPhoneme("きゅ");
    ASSERT_EQ(static_cast<int>(seq.size()), 4);
    ASSERT_NEAR(seq.back().params.f1, kVowelU.f1, 0.01);  // 350
    ASSERT_NEAR(seq.back().params.f2, kVowelU.f2, 0.01);
}

// 6. きょ: K plosive + vowel O
REGISTER_TEST(kyo_youon) {
    auto seq = textToPhoneme("きょ");
    ASSERT_EQ(static_cast<int>(seq.size()), 4);
    ASSERT_NEAR(seq.back().params.f1, kVowelO.f1, 0.01);  // 500
    ASSERT_NEAR(seq.back().params.f2, kVowelO.f2, 0.01);
}

// --- Special character tests ---

// 7. 促音: かっぱ has kSilence in the middle
REGISTER_TEST(sokuon_kappa) {
    auto seq = textToPhoneme("かっぱ");
    // か(4) + っ(1) + ぱ(4) = 9 entries
    ASSERT_EQ(static_cast<int>(seq.size()), 9);
    // Entry index 4 (after か's 4 entries) should be silence
    ASSERT_NEAR(seq[4].params.gain, kSilence.gain, 0.01);  // 0.0
    ASSERT_EQ(seq[4].duration_samples, kSokuonSamples);     // 5292
}

// 8. 撥音: かんな has kNasalN for ん
REGISTER_TEST(hatsuon_kanna) {
    auto seq = textToPhoneme("かんな");
    // か(4) + ん(1) + な(2) = 7 entries
    ASSERT_EQ(static_cast<int>(seq.size()), 7);
    // Entry index 4 should be hatsuon
    ASSERT_NEAR(seq[4].params.f2, kNasalN.f2, 0.01);       // 1700
    ASSERT_EQ(seq[4].duration_samples, kHatsuonSamples);    // 3528
}

// 9. 長音: かー extends last vowel
REGISTER_TEST(chouon_ka) {
    auto seq = textToPhoneme("かー");
    // か(4) + ー(1) = 5 entries
    ASSERT_EQ(static_cast<int>(seq.size()), 5);
    ASSERT_NEAR(seq.back().params.f1, kVowelA.f1, 0.01);   // 800
    ASSERT_EQ(seq.back().duration_samples, ms2s(120));       // 5292
}

// 10. 複合: きょう = きょ(4) + う(1)
REGISTER_TEST(kyou_composite) {
    auto seq = textToPhoneme("きょう");
    // きょ(4) + う(1) = 5 entries
    ASSERT_EQ(static_cast<int>(seq.size()), 5);
    // きょ last entry = vowel O
    ASSERT_NEAR(seq[3].params.f1, kVowelO.f1, 0.01);
    // う = vowel U
    ASSERT_NEAR(seq[4].params.f1, kVowelU.f1, 0.01);
}

// 11. 連続拗音: しゃしゅしょ = 3 x (SH+vowel) = 6 entries
REGISTER_TEST(sha_shu_sho) {
    auto seq = textToPhoneme("しゃしゅしょ");
    ASSERT_EQ(static_cast<int>(seq.size()), 6);
    // Each pair: fricative SH + vowel
    ASSERT_NEAR(seq[0].params.f2, kFricSh.f2, 0.01);
    ASSERT_NEAR(seq[1].params.f1, kVowelA.f1, 0.01);  // a
    ASSERT_NEAR(seq[2].params.f2, kFricSh.f2, 0.01);
    ASSERT_NEAR(seq[3].params.f1, kVowelU.f1, 0.01);  // u
    ASSERT_NEAR(seq[4].params.f2, kFricSh.f2, 0.01);
    ASSERT_NEAR(seq[5].params.f1, kVowelO.f1, 0.01);  // o
}

// 12. 拗音なし i列: き = K + vowel I (not youon)
REGISTER_TEST(ki_no_youon) {
    auto seq = textToPhoneme("き");
    ASSERT_EQ(static_cast<int>(seq.size()), 4);
    ASSERT_NEAR(seq.back().params.f1, kVowelI.f1, 0.01);  // 300
    ASSERT_NEAR(seq.back().params.f2, kVowelI.f2, 0.01);
}

int main() {
    return run_all_tests("YouonSpecial");
}
