// test_integration.cpp — Full pipeline integration tests
#define TEST_BUILD
#include "../formant.cpp"
#include "../test_framework.h"
#include <filesystem>
#include <numeric>

// RMS計算ヘルパー
static double compute_rms(const std::vector<int16_t>& samples, int offset, int count) {
    double sum_sq = 0.0;
    for (int i = offset; i < offset + count; ++i) {
        double s = static_cast<double>(samples[i]);
        sum_sq += s * s;
    }
    return std::sqrt(sum_sq / count);
}

// 1. 5母音シーケンス(A->I->U->E->O、各5292サンプル)を合成し、出力サイズが26460か
REGISTER_TEST(five_vowel_sequence_size) {
    Synthesizer synth;
    std::vector<PhonemeEntry> sequence = {
        {kVowelA, 5292},
        {kVowelI, 5292},
        {kVowelU, 5292},
        {kVowelE, 5292},
        {kVowelO, 5292},
    };
    std::vector<int16_t> output;
    synth.synthesize(sequence, output);
    ASSERT_EQ(static_cast<int>(output.size()), 5 * 5292);
}

// 2. CV音節(は=kFricH_A 3528サンプル + kVowelA 5292サンプル)を合成し、出力サイズが8820か
REGISTER_TEST(cv_syllable_ha_size) {
    Synthesizer synth;
    std::vector<PhonemeEntry> sequence = {
        {kFricH_A, 3528},
        {kVowelA,  5292},
    };
    std::vector<int16_t> output;
    synth.synthesize(sequence, output);
    ASSERT_EQ(static_cast<int>(output.size()), 3528 + 5292);
}

// 3. 破裂音CV(か)をmakePlosiveCVで生成→合成し、出力サイズが正しいか
REGISTER_TEST(plosive_cv_ka_size) {
    auto ka = makePlosiveCV(80, 10, 30, kSilence, kBurstK_A, kVotK_A, kVowelA, 120);

    // 期待されるサンプル数を計算
    int expected = ms2s(80) + ms2s(10) + ms2s(30) + ms2s(120);

    // エントリ数の確認: closure + burst + vot + vowel = 4
    ASSERT_EQ(static_cast<int>(ka.size()), 4);

    // 合計duration確認
    int total_dur = 0;
    for (auto& e : ka) total_dur += e.duration_samples;
    ASSERT_EQ(total_dur, expected);

    // 合成して出力サイズ確認
    Synthesizer synth;
    std::vector<int16_t> output;
    synth.synthesize(ka, output);
    ASSERT_EQ(static_cast<int>(output.size()), expected);
}

// 4. 促音テスト: kSilenceのkSokuonSamples区間を含むシーケンスの出力サイズが正しく、
//    促音区間のRMSが前後の母音区間より十分小さいか
REGISTER_TEST(sokuon_silence) {
    // 母音A → 促音(無音) → 母音A のシーケンス
    std::vector<PhonemeEntry> sequence = {
        {kVowelA,  5292},
        {kSilence, kSokuonSamples},
        {kVowelA,  5292},
    };
    Synthesizer synth;
    std::vector<int16_t> output;
    synth.synthesize(sequence, output);

    ASSERT_EQ(static_cast<int>(output.size()), 5292 + kSokuonSamples + 5292);

    // 促音単独で合成: フィルタ残響なしなら gain=0 で完全無音に近いはず
    Synthesizer synth2;
    std::vector<PhonemeEntry> silence_only = {{kSilence, kSokuonSamples}};
    std::vector<int16_t> sil_output;
    synth2.synthesize(silence_only, sil_output);

    ASSERT_EQ(static_cast<int>(sil_output.size()), kSokuonSamples);
    double rms_sil = compute_rms(sil_output, 0, kSokuonSamples);
    // gain=0 の音源単独では RMS はほぼ 0
    ASSERT_LT(rms_sil, 50.0);

    // また、フルシーケンスでも促音後の母音は鳴っている（合成が途切れていない）
    double rms_after = compute_rms(output, 5292 + kSokuonSamples, 5292);
    ASSERT_GT(rms_after, 0.0);
}

// 5. 撥音テスト: kNasalN kHatsuonSamples の区間を含むシーケンスが正しいサイズで合成されるか
REGISTER_TEST(hatsuon_sequence_size) {
    std::vector<PhonemeEntry> sequence = {
        {kVowelA,  5292},
        {kNasalN,  kHatsuonSamples},
        {kVowelA,  5292},
    };
    Synthesizer synth;
    std::vector<int16_t> output;
    synth.synthesize(sequence, output);

    int expected = 5292 + kHatsuonSamples + 5292;
    ASSERT_EQ(static_cast<int>(output.size()), expected);

    // 撥音区間のRMSが0でないことも確認（鳴っているはず）
    double rms = compute_rms(output, 5292, kHatsuonSamples);
    ASSERT_GT(rms, 0.0);
}

// 6. フルシーケンス→WAV書き出し→ファイルサイズが正しいか（44バイトヘッダ + サンプル数*2）
REGISTER_TEST(wav_file_size) {
    const char* path = "/tmp/test_integration_wav.wav";

    // 小さなシーケンスを合成
    Synthesizer synth;
    std::vector<PhonemeEntry> sequence = {
        {kVowelA, 4410},
        {kFricS,  2205},
    };
    std::vector<int16_t> output;
    synth.synthesize(sequence, output);

    int num_samples = static_cast<int>(output.size());
    ASSERT_EQ(num_samples, 4410 + 2205);

    // WAV書き出し
    bool ok = writeWav(path, output, static_cast<int>(kSampleRate));
    ASSERT_TRUE(ok);

    // ファイルサイズ確認: 44バイトヘッダ + サンプル数 * 2
    auto fsize = std::filesystem::file_size(path);
    auto expected = static_cast<std::uintmax_t>(44 + num_samples * 2);
    ASSERT_EQ(fsize, expected);

    // クリーンアップ
    std::filesystem::remove(path);
}

// 7. 2回合成して同じ結果か（決定的であること確認）
REGISTER_TEST(deterministic_output) {
    auto run_synth = []() {
        Synthesizer synth;
        std::vector<PhonemeEntry> sequence = {
            {kFricH_A, 3528},
            {kVowelA,  5292},
            {kNasalN,  3528},
            {kVowelI,  5292},
        };
        std::vector<int16_t> output;
        synth.synthesize(sequence, output);
        return output;
    };

    auto out1 = run_synth();
    auto out2 = run_synth();

    ASSERT_EQ(static_cast<int>(out1.size()), static_cast<int>(out2.size()));
    for (int i = 0; i < static_cast<int>(out1.size()); ++i) {
        ASSERT_EQ(out1[i], out2[i]);
    }
}

// 8. 破擦音 ち(tɕi) をフルパイプラインで合成→WAV書き出し→ファイルサイズ確認→削除
REGISTER_TEST(affricate_chi_full_pipeline) {
    const char* path = "/tmp/test_integration_ms5_chi.wav";

    auto chi = makePlosiveCV(70, 5, 80, kSilence, kBurstTCh, kFricSh, kVowelI, 120);

    // 期待サンプル数
    int expected_samples = ms2s(70) + ms2s(5) + ms2s(80) + ms2s(120);

    Synthesizer synth;
    std::vector<int16_t> output;
    synth.synthesize(chi, output);
    ASSERT_EQ(static_cast<int>(output.size()), expected_samples);

    // WAV書き出し
    bool ok = writeWav(path, output, static_cast<int>(kSampleRate));
    ASSERT_TRUE(ok);

    // ファイルサイズ確認: 44バイトヘッダ + サンプル数 * 2
    auto fsize = std::filesystem::file_size(path);
    auto expected_fsize = static_cast<std::uintmax_t>(44 + expected_samples * 2);
    ASSERT_EQ(fsize, expected_fsize);

    std::filesystem::remove(path);
}

// 9. さ行5音節(さしすせそ)を合成し出力サイズが正しいか
REGISTER_TEST(sa_row_sequence) {
    std::vector<PhonemeEntry> sequence;

    // さ [sa]
    sequence.push_back({kFricS, ms2s(120)});
    sequence.push_back({kVowelA, ms2s(120)});
    // し [ɕi]
    sequence.push_back({kFricSh, ms2s(120)});
    sequence.push_back({kVowelI, ms2s(120)});
    // す [sɯ]
    sequence.push_back({kFricS, ms2s(120)});
    sequence.push_back({kVowelU, ms2s(120)});
    // せ [se]
    sequence.push_back({kFricS, ms2s(120)});
    sequence.push_back({kVowelE, ms2s(120)});
    // そ [so]
    sequence.push_back({kFricS, ms2s(120)});
    sequence.push_back({kVowelO, ms2s(120)});

    int expected = 10 * ms2s(120); // 10 segments x 120ms each

    Synthesizer synth;
    std::vector<int16_t> output;
    synth.synthesize(sequence, output);
    ASSERT_EQ(static_cast<int>(output.size()), expected);
}

// 10. は行5音節(はひふへほ)を合成し出力サイズが正しいか
REGISTER_TEST(ha_row_sequence) {
    std::vector<PhonemeEntry> sequence;

    // は [ha]
    sequence.push_back({kFricH_A, ms2s(80)});
    sequence.push_back({kVowelA, ms2s(120)});
    // ひ [çi]
    sequence.push_back({kFricChi, ms2s(100)});
    sequence.push_back({kVowelI, ms2s(120)});
    // ふ [ɸɯ]
    sequence.push_back({kFricPhi, ms2s(100)});
    sequence.push_back({kVowelU, ms2s(120)});
    // へ [he]
    sequence.push_back({kFricH_E, ms2s(80)});
    sequence.push_back({kVowelE, ms2s(120)});
    // ほ [ho]
    sequence.push_back({kFricH_O, ms2s(80)});
    sequence.push_back({kVowelO, ms2s(120)});

    int expected = 3 * ms2s(80) + 2 * ms2s(100) + 5 * ms2s(120);

    Synthesizer synth;
    std::vector<int16_t> output;
    synth.synthesize(sequence, output);
    ASSERT_EQ(static_cast<int>(output.size()), expected);
}

// 11. ざ行5音節(ざじずぜぞ)を合成し出力サイズが正しいか
REGISTER_TEST(za_row_sequence) {
    std::vector<PhonemeEntry> sequence;

    // ざ [dza]
    append(sequence, makePlosiveCV(40, 5, 50, kVoiceBar, kBurstTs, kFricZ, kVowelA, 120));
    // じ [dʑi]
    append(sequence, makePlosiveCV(40, 5, 60, kVoiceBar, kBurstTCh, kFricZh, kVowelI, 120));
    // ず [dzɯ]
    append(sequence, makePlosiveCV(40, 5, 50, kVoiceBar, kBurstTs, kFricZ, kVowelU, 120));
    // ぜ [dze]
    append(sequence, makePlosiveCV(40, 5, 50, kVoiceBar, kBurstTs, kFricZ, kVowelE, 120));
    // ぞ [dzo]
    append(sequence, makePlosiveCV(40, 5, 50, kVoiceBar, kBurstTs, kFricZ, kVowelO, 120));

    // 各音節のサンプル数を計算
    int za = ms2s(40) + ms2s(5) + ms2s(50) + ms2s(120); // ざ,ず,ぜ,ぞ
    int ji = ms2s(40) + ms2s(5) + ms2s(60) + ms2s(120); // じ
    int expected = 4 * za + ji;

    Synthesizer synth;
    std::vector<int16_t> output;
    synth.synthesize(sequence, output);
    ASSERT_EQ(static_cast<int>(output.size()), expected);
}

// 12. Mixed音源を含むシーケンスを2回合成して同じ結果か（決定的であること確認）
REGISTER_TEST(mixed_source_deterministic) {
    auto run_synth = []() {
        Synthesizer synth;
        std::vector<PhonemeEntry> sequence;
        // ざ行(Mixed音源)を含むシーケンス
        append(sequence, makePlosiveCV(40, 5, 50, kVoiceBar, kBurstTs, kFricZ, kVowelA, 120));
        sequence.push_back({kFricZh, ms2s(80)});
        sequence.push_back({kVowelI, ms2s(120)});
        std::vector<int16_t> output;
        synth.synthesize(sequence, output);
        return output;
    };

    auto out1 = run_synth();
    auto out2 = run_synth();

    ASSERT_EQ(static_cast<int>(out1.size()), static_cast<int>(out2.size()));
    for (int i = 0; i < static_cast<int>(out1.size()); ++i) {
        ASSERT_EQ(out1[i], out2[i]);
    }
}

int main() {
    return run_all_tests("Integration");
}
