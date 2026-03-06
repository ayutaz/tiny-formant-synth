// test_za_row.cpp — Tests for za-row (voiced affricates: za/ji/zu/ze/zo)
#define TEST_BUILD
#include "../formant.cpp"
#include "../test_framework.h"
#include <cmath>
#include <numeric>

// ざ行の makePlosiveCV 呼び出しヘルパー
static auto make_za() { return makePlosiveCV(40, 5, 50, kVoiceBar, kBurstTs, kFricZ, kVowelA, 120); }
static auto make_ji() { return makePlosiveCV(40, 5, 60, kVoiceBar, kBurstTCh, kFricZh, kVowelI, 120); }
static auto make_zu() { return makePlosiveCV(40, 5, 50, kVoiceBar, kBurstTs, kFricZ, kVowelU, 120); }
static auto make_ze() { return makePlosiveCV(40, 5, 50, kVoiceBar, kBurstTs, kFricZ, kVowelE, 120); }
static auto make_zo() { return makePlosiveCV(40, 5, 50, kVoiceBar, kBurstTs, kFricZ, kVowelO, 120); }

// 1. ざ用makePlosiveCVが4エントリ返すか (closure + burst + fric + vowel)
REGISTER_TEST(za_entry_count) {
    auto za = make_za();
    ASSERT_EQ(static_cast<int>(za.size()), 4);
}

// 2. じ用makePlosiveCVが4エントリ返すか
REGISTER_TEST(ji_entry_count) {
    auto ji = make_ji();
    ASSERT_EQ(static_cast<int>(ji.size()), 4);
}

// 3. ざ音節を合成し出力サイズが正しいか
REGISTER_TEST(za_synthesize) {
    auto za = make_za();
    int expected = ms2s(40) + ms2s(5) + ms2s(50) + ms2s(120);
    Synthesizer synth;
    std::vector<int16_t> output;
    synth.synthesize(za, output);
    ASSERT_EQ(static_cast<int>(output.size()), expected);
}

// 4. じ音節を合成し出力サイズが正しいか
REGISTER_TEST(ji_synthesize) {
    auto ji = make_ji();
    int expected = ms2s(40) + ms2s(5) + ms2s(60) + ms2s(120);
    Synthesizer synth;
    std::vector<int16_t> output;
    synth.synthesize(ji, output);
    ASSERT_EQ(static_cast<int>(output.size()), expected);
}

// 5. ず音節を合成し出力サイズが正しいか
REGISTER_TEST(zu_synthesize) {
    auto zu = make_zu();
    int expected = ms2s(40) + ms2s(5) + ms2s(50) + ms2s(120);
    Synthesizer synth;
    std::vector<int16_t> output;
    synth.synthesize(zu, output);
    ASSERT_EQ(static_cast<int>(output.size()), expected);
}

// 6. ぜ音節を合成し出力サイズが正しいか
REGISTER_TEST(ze_synthesize) {
    auto ze = make_ze();
    int expected = ms2s(40) + ms2s(5) + ms2s(50) + ms2s(120);
    Synthesizer synth;
    std::vector<int16_t> output;
    synth.synthesize(ze, output);
    ASSERT_EQ(static_cast<int>(output.size()), expected);
}

// 7. ぞ音節を合成し出力サイズが正しいか
REGISTER_TEST(zo_synthesize) {
    auto zo = make_zo();
    int expected = ms2s(40) + ms2s(5) + ms2s(50) + ms2s(120);
    Synthesizer synth;
    std::vector<int16_t> output;
    synth.synthesize(zo, output);
    ASSERT_EQ(static_cast<int>(output.size()), expected);
}

// 8. じの摩擦区間がkFricZh(F2=3800)を使用
REGISTER_TEST(ji_uses_kFricZh) {
    auto ji = make_ji();
    // ji[2] = 摩擦区間 (vot_params = kFricZh)
    ASSERT_NEAR(ji[2].params.f2, 3800.0, 0.01);
    ASSERT_NEAR(ji[2].params.bw2, 2500.0, 0.01);
    ASSERT_EQ(static_cast<int>(ji[2].params.source), static_cast<int>(SourceType::Mixed));
}

// 9. ざの摩擦区間がkFricZ(F2=5500)を使用
REGISTER_TEST(za_uses_kFricZ) {
    auto za = make_za();
    // za[2] = 摩擦区間 (vot_params = kFricZ)
    ASSERT_NEAR(za[2].params.f2, 5500.0, 0.01);
    ASSERT_NEAR(za[2].params.bw2, 3000.0, 0.01);
    ASSERT_EQ(static_cast<int>(za[2].params.source), static_cast<int>(SourceType::Mixed));
}

// 10. ざ行の閉鎖区間が kVoiceBar(有声)を使用
REGISTER_TEST(all_za_row_voiced) {
    auto za = make_za();
    auto ji = make_ji();
    auto zu = make_zu();
    auto ze = make_ze();
    auto zo = make_zo();

    // 各音節の先頭エントリ(閉鎖区間)がkVoiceBarと一致
    for (auto* row : {&za, &ji, &zu, &ze, &zo}) {
        auto& closure = (*row)[0].params;
        ASSERT_NEAR(closure.f1, kVoiceBar.f1, 0.01);
        ASSERT_NEAR(closure.f2, kVoiceBar.f2, 0.01);
        ASSERT_NEAR(closure.gain, kVoiceBar.gain, 0.001);
        ASSERT_EQ(static_cast<int>(closure.source), static_cast<int>(SourceType::Impulse));
    }
}

int main() {
    return run_all_tests("ZaRow");
}
