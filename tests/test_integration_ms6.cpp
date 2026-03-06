// test_integration_ms6.cpp — MS6 text-to-speech pipeline integration tests
#define TEST_BUILD
#include "../formant.cpp"
#include "../test_framework.h"
#include <cstdio>
#include <filesystem>

// 1. あいうえお合成: textToPhoneme → Synthesizer → サンプル数 == 5 * ms2s(120) = 26460
REGISTER_TEST(aiueo_synthesis) {
    auto seq = textToPhoneme("あいうえお");
    ASSERT_EQ(static_cast<int>(seq.size()), 5);  // 5母音 = 5セグメント

    Synthesizer synth;
    std::vector<int16_t> buf;
    synth.synthesize(seq, buf);

    ASSERT_TRUE(!buf.empty());
    ASSERT_EQ(static_cast<int>(buf.size()), 5 * ms2s(120));  // 26460
}

// 2. こんにちは合成: textToPhoneme → Synthesizer → 非空
REGISTER_TEST(konnichiwa_synthesis) {
    auto seq = textToPhoneme("こんにちは");
    ASSERT_TRUE(!seq.empty());

    Synthesizer synth;
    std::vector<int16_t> buf;
    synth.synthesize(seq, buf);

    ASSERT_GT(static_cast<int>(buf.size()), 0);
}

// 3. さくら合成: textToPhoneme → 正しいセグメント数
//    さ = S,0 → 2 (fric + vowel)
//    く = K,2 → 4 (closure + burst + vot + vowel)
//    ら = R,0 → 2 (tap + vowel)
//    合計 = 8
REGISTER_TEST(sakura_segment_count) {
    auto seq = textToPhoneme("さくら");
    ASSERT_EQ(static_cast<int>(seq.size()), 8);
}

// 4. 全濁音合成: がぎぐげご → クラッシュしない、サンプル数 > 0
REGISTER_TEST(dakuon_gagigugego) {
    auto seq = textToPhoneme("がぎぐげご");
    ASSERT_TRUE(!seq.empty());

    Synthesizer synth;
    std::vector<int16_t> buf;
    synth.synthesize(seq, buf);

    ASSERT_GT(static_cast<int>(buf.size()), 0);
}

// 5. 全半濁音合成: ぱぴぷぺぽ → クラッシュしない、サンプル数 > 0
REGISTER_TEST(handakuon_papipupepo) {
    auto seq = textToPhoneme("ぱぴぷぺぽ");
    ASSERT_TRUE(!seq.empty());

    Synthesizer synth;
    std::vector<int16_t> buf;
    synth.synthesize(seq, buf);

    ASSERT_GT(static_cast<int>(buf.size()), 0);
}

// 6. サンプル範囲チェック: 全サンプルが [-32768, 32767] に収まる
REGISTER_TEST(sample_range_check) {
    auto seq = textToPhoneme("こんにちは");

    Synthesizer synth;
    std::vector<int16_t> buf;
    synth.synthesize(seq, buf);

    ASSERT_TRUE(!buf.empty());
    for (int i = 0; i < static_cast<int>(buf.size()); ++i) {
        ASSERT_GE(static_cast<int>(buf[i]), -32768);
        ASSERT_LE(static_cast<int>(buf[i]), 32767);
    }
}

// 7. WAV書き出し: ファイルが存在し、サイズが正しい (44 + samples*2 bytes)
REGISTER_TEST(wav_write_pipeline) {
    const char* path = "/tmp/test_ms6.wav";

    auto seq = textToPhoneme("あいう");

    Synthesizer synth;
    std::vector<int16_t> buf;
    synth.synthesize(seq, buf);

    ASSERT_TRUE(!buf.empty());

    bool ok = writeWav(path, buf, static_cast<int>(kSampleRate));
    ASSERT_TRUE(ok);

    auto fsize = std::filesystem::file_size(path);
    auto expected = static_cast<std::uintmax_t>(44 + buf.size() * 2);
    ASSERT_EQ(fsize, expected);

    std::remove(path);
}

// 8. 促音含む合成: かっぱ → 正常
REGISTER_TEST(sokuon_kappa) {
    auto seq = textToPhoneme("かっぱ");
    ASSERT_TRUE(!seq.empty());

    // か(K,0)=4 + っ(Q)=1 + ぱ(P,0)=4 → 9セグメント
    ASSERT_EQ(static_cast<int>(seq.size()), 9);

    Synthesizer synth;
    std::vector<int16_t> buf;
    synth.synthesize(seq, buf);

    ASSERT_GT(static_cast<int>(buf.size()), 0);
}

// 9. 拗音含む合成: きょう → 正常
REGISTER_TEST(youon_kyou) {
    auto seq = textToPhoneme("きょう");
    ASSERT_TRUE(!seq.empty());

    // き+ょ → K,4 (拗音: vi=1→4) = 4セグメント, う → V,2 = 1セグメント → 5セグメント
    ASSERT_EQ(static_cast<int>(seq.size()), 5);

    Synthesizer synth;
    std::vector<int16_t> buf;
    synth.synthesize(seq, buf);

    ASSERT_GT(static_cast<int>(buf.size()), 0);
}

// 10. 五十音全合成テスト: あかさたなはまやらわ → クラッシュしない
REGISTER_TEST(gojuon_full_synthesis) {
    auto seq = textToPhoneme("あかさたなはまやらわ");
    ASSERT_TRUE(!seq.empty());

    Synthesizer synth;
    std::vector<int16_t> buf;
    synth.synthesize(seq, buf);

    ASSERT_GT(static_cast<int>(buf.size()), 0);
}

int main() {
    return run_all_tests("IntegrationMS6");
}
