// formant.cpp — MS4: Plosives /p,b,t,d,k,g/ + sokuon + hatsuon
// Build: make  (expects -std=c++20 -O2 -Wall -Wextra)

#include <cstdint>
#include <cmath>
#include <vector>
#include <fstream>
#include <iostream>
#include <numbers>
#include <array>
#include <algorithm>

// ============ Constants & Types ============

constexpr double kSampleRate  = 44100.0;
constexpr double kF0          = 160.0;
constexpr double kAmplitude   = 0.9 * 32767.0;
constexpr double kPi          = std::numbers::pi;
constexpr int    kFrameSize   = 220;           // 5 ms at 44.1 kHz

enum class SourceType { Impulse, Noise };

struct FormantParams {
    double f1, f2, f3;
    double bw1, bw2, bw3;
    double gain = 1.0;
    SourceType source = SourceType::Impulse;
};

// FormantParams 間の線形補間
static FormantParams lerp(const FormantParams& a, const FormantParams& b, double t) {
    return {
        a.f1 + (b.f1 - a.f1) * t,
        a.f2 + (b.f2 - a.f2) * t,
        a.f3 + (b.f3 - a.f3) * t,
        a.bw1 + (b.bw1 - a.bw1) * t,
        a.bw2 + (b.bw2 - a.bw2) * t,
        a.bw3 + (b.bw3 - a.bw3) * t,
        a.gain + (b.gain - a.gain) * t,
        a.source,  // 音源タイプは補間しない（現在のセグメントの値を使用）
    };
}

// ============ Resonator ============

struct Resonator {
    double z1 = 0, z2 = 0;
    double a0 = 0, b1 = 0, b2 = 0;

    void set(double freq, double bw, double fs) {
        double R = std::exp(-kPi * bw / fs);
        b1 = -2.0 * R * std::cos(2.0 * kPi * freq / fs);
        b2 = R * R;
        a0 = 1.0 + b1 + b2;
    }

    double process(double input) {
        double y = a0 * input - b1 * z1 - b2 * z2;
        z2 = z1;
        z1 = y;
        return y;
    }

    void reset() { z1 = z2 = 0; }
};

// ============ Source Generation ============

struct NoiseGen {
    uint32_t seed = 22695477;
    double next() {
        seed = seed * 1664525 + 1013904223;  // LCG
        return static_cast<double>(static_cast<int32_t>(seed)) / 2147483648.0;
    }
};

struct ImpulseTrain {
    double phase = 0.0;

    double next(double f0, double fs) {
        phase += f0 / fs;
        if (phase >= 1.0) {
            phase -= 1.0;
            return 1.0;
        }
        return 0.0;
    }
};

// ============ Synthesizer ============

struct PhonemeEntry {
    FormantParams params;
    int duration_samples;
};

class Synthesizer {
public:
    void synthesize(const std::vector<PhonemeEntry>& sequence,
                    std::vector<int16_t>& output) {
        int totalSamples = 0;
        for (auto& e : sequence) totalSamples += e.duration_samples;
        output.resize(totalSamples);

        int pos = 0;
        for (std::size_t seg = 0; seg < sequence.size(); ++seg) {
            const auto& cur = sequence[seg].params;
            const auto& nxt = (seg + 1 < sequence.size())
                                  ? sequence[seg + 1].params : cur;
            int dur = sequence[seg].duration_samples;

            for (int n = 0; n < dur; ++n) {
                // フレーム境界でフィルタ係数更新
                if (n % kFrameSize == 0) {
                    double t = static_cast<double>(n) / dur;
                    double blend = (t > 0.7) ? (t - 0.7) / 0.3 : 0.0;
                    auto p = lerp(cur, nxt, blend);

                    filters_[0].set(p.f1, p.bw1, kSampleRate);
                    filters_[1].set(p.f2, p.bw2, kSampleRate);
                    filters_[2].set(p.f3, p.bw3, kSampleRate);
                    currentGain_ = p.gain;
                }

                // 音源生成
                double s = (cur.source == SourceType::Noise)
                    ? noise_.next()
                    : impulse_.next(kF0, kSampleRate);

                // カスケードフィルタ
                for (auto& f : filters_) s = f.process(s);

                // 出力
                double out = s * kAmplitude * currentGain_;
                out = std::clamp(out, -32768.0, 32767.0);
                output[pos++] = static_cast<int16_t>(out);
            }
        }
    }

private:
    std::array<Resonator, 3> filters_{};
    ImpulseTrain impulse_;
    NoiseGen noise_;
    double currentGain_ = 1.0;
};

// ============ Phoneme Data ============

constexpr FormantParams kVowelA = {800, 1200, 2600, 80, 100, 120};
constexpr FormantParams kVowelI = {300, 2300, 3000, 80, 120, 150};
constexpr FormantParams kVowelU = {350, 1300, 2500, 80, 100, 120};
constexpr FormantParams kVowelE = {500, 1900, 2600, 80, 100, 120};
constexpr FormantParams kVowelO = {500,  800, 2400, 80, 100, 120};

// 半母音（音源:インパルス、遷移のみで実現）
constexpr FormantParams kSemiJ = {280, 2300, 3000, 80, 100, 120, 1.0, SourceType::Impulse};
constexpr FormantParams kSemiW = {320, 750,  2300, 80, 100, 120, 1.0, SourceType::Impulse};

// 弾き音（音源:インパルス、短い持続、低ゲイン）
constexpr FormantParams kTapR  = {350, 1500, 2500, 80, 120, 150, 0.3, SourceType::Impulse};

// 鼻音（音源:インパルス、F1=250Hz広帯域、低ゲイン）
constexpr FormantParams kNasalM = {250, 1000, 2200, 180, 200, 250, 0.3, SourceType::Impulse};
constexpr FormantParams kNasalN = {250, 1700, 2600, 180, 200, 250, 0.3, SourceType::Impulse};

// 摩擦音（音源:ノイズ）
// /h/ は後続母音「あ」のフォルマントに近づけた値で音源だけノイズに切り替え
constexpr FormantParams kFricH_A = {800, 1200, 2600, 200, 300, 400, 0.15, SourceType::Noise};
// /s/ は高域ノイズが特徴（F1=200で低域を広く減衰）
constexpr FormantParams kFricS   = {200, 5500, 7500, 500, 3000, 2000, 0.4, SourceType::Noise};

// 無音（gain=0、低周波ダミー値）
constexpr FormantParams kSilence = {100, 100, 100, 100, 100, 100, 0.0, SourceType::Impulse};

// --- 破裂音パラメータ ---

// 有声破裂音の閉鎖区間: voice bar（F1≈200Hz 低振幅インパルス）
constexpr FormantParams kVoiceBar = {200, 200, 200, 100, 200, 300, 0.08, SourceType::Impulse};

// バーストパラメータ（短いノイズ）
constexpr FormantParams kBurstP   = {300, 1000, 2300, 500, 1500, 2000, 0.30, SourceType::Noise};
constexpr FormantParams kBurstT   = {300, 4000, 5000, 500, 2000, 2000, 0.35, SourceType::Noise};
constexpr FormantParams kBurstK_A = {300, 1800, 2600, 500, 2000, 2000, 0.30, SourceType::Noise};

// VOT/気息パラメータ（後続母音 /a/ 向け — 母音フォルマントベースにBW広・gain小）
constexpr FormantParams kVotP_A = {800, 1200, 2600, 300, 400, 500, 0.10, SourceType::Noise};
constexpr FormantParams kVotT_A = {800, 1200, 2600, 300, 400, 500, 0.10, SourceType::Noise};
constexpr FormantParams kVotK_A = {800, 1200, 2600, 300, 400, 500, 0.10, SourceType::Noise};

// 促音: 120ms の無音区間
constexpr int kSokuonSamples = 5292;  // 120 ms

// 撥音: /n/ を 80ms で使用
constexpr int kHatsuonSamples = 3528; // 80 ms

// --- ヘルパー関数 ---

// ミリ秒→サンプル数変換
inline constexpr int ms2s(int ms) {
    return static_cast<int>(kSampleRate * ms / 1000.0);
}

// 破裂音CV の PhonemeEntry 列を生成
static std::vector<PhonemeEntry> makePlosiveCV(
    int closure_ms,
    int burst_ms,
    int vot_ms,
    const FormantParams& closure_params,
    const FormantParams& burst_params,
    const FormantParams& vot_params,
    const FormantParams& vowel,
    int vowel_ms)
{
    std::vector<PhonemeEntry> result;
    if (closure_ms > 0) result.push_back({closure_params, ms2s(closure_ms)});
    if (burst_ms > 0)   result.push_back({burst_params,   ms2s(burst_ms)});
    if (vot_ms > 0)     result.push_back({vot_params,     ms2s(vot_ms)});
    result.push_back({vowel, ms2s(vowel_ms)});
    return result;
}

// シーケンスに複数エントリを追加
static void append(std::vector<PhonemeEntry>& seq, const std::vector<PhonemeEntry>& entries) {
    seq.insert(seq.end(), entries.begin(), entries.end());
}

// ============ WAV Writer ============

static bool writeWav(const char* filename,
                     const std::vector<int16_t>& samples,
                     int sampleRate) {
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        std::cerr << "Error: cannot open " << filename << "\n";
        return false;
    }

    auto write16 = [&](uint16_t v) {
        ofs.write(reinterpret_cast<const char*>(&v), 2);
    };
    auto write32 = [&](uint32_t v) {
        ofs.write(reinterpret_cast<const char*>(&v), 4);
    };

    uint32_t dataSize = static_cast<uint32_t>(samples.size()) * 2;
    uint32_t fileSize = 36 + dataSize;

    // RIFF header
    ofs.write("RIFF", 4);
    write32(fileSize);
    ofs.write("WAVE", 4);

    // fmt chunk
    ofs.write("fmt ", 4);
    write32(16);                                          // chunk size
    write16(1);                                           // PCM
    write16(1);                                           // mono
    write32(static_cast<uint32_t>(sampleRate));           // sample rate
    write32(static_cast<uint32_t>(sampleRate) * 2);       // byte rate
    write16(2);                                           // block align
    write16(16);                                          // bits per sample

    // data chunk
    ofs.write("data", 4);
    write32(dataSize);
    ofs.write(reinterpret_cast<const char*>(samples.data()),
              static_cast<std::streamsize>(dataSize));

    return true;
}

// ============ Main ============

int main() {
    constexpr int kPause     = 882;    // 20 ms
    constexpr int kLongPause = 13230;  // 300 ms（テスト間の区切り）

    std::vector<PhonemeEntry> sequence;

    // --- テスト1: か た ぱ（無声破裂音 /k,t,p/）---
    append(sequence, makePlosiveCV(80, 10, 30, kSilence, kBurstK_A, kVotK_A, kVowelA, 120));
    sequence.push_back({kSilence, kPause});
    append(sequence, makePlosiveCV(70, 10, 20, kSilence, kBurstT, kVotT_A, kVowelA, 120));
    sequence.push_back({kSilence, kPause});
    append(sequence, makePlosiveCV(70, 10, 15, kSilence, kBurstP, kVotP_A, kVowelA, 120));
    sequence.push_back({kSilence, kLongPause});

    // --- テスト2: が だ ば（有声破裂音 /g,d,b/）---
    append(sequence, makePlosiveCV(60, 5, 0, kVoiceBar, kBurstK_A, kVotK_A, kVowelA, 120));
    sequence.push_back({kSilence, kPause});
    append(sequence, makePlosiveCV(50, 5, 0, kVoiceBar, kBurstT, kVotT_A, kVowelA, 120));
    sequence.push_back({kSilence, kPause});
    append(sequence, makePlosiveCV(50, 5, 0, kVoiceBar, kBurstP, kVotP_A, kVowelA, 120));
    sequence.push_back({kSilence, kLongPause});

    // --- テスト3: かっぱ（促音テスト）---
    append(sequence, makePlosiveCV(80, 10, 30, kSilence, kBurstK_A, kVotK_A, kVowelA, 100));
    sequence.push_back({kSilence, kSokuonSamples});  // 促音 /Q/
    append(sequence, makePlosiveCV(70, 10, 15, kSilence, kBurstP, kVotP_A, kVowelA, 120));
    sequence.push_back({kSilence, kLongPause});

    // --- テスト4: かんな（撥音テスト）---
    append(sequence, makePlosiveCV(80, 10, 30, kSilence, kBurstK_A, kVotK_A, kVowelA, 100));
    sequence.push_back({kNasalN, kHatsuonSamples});   // 撥音 /N/
    sequence.push_back({kNasalN, ms2s(80)});           // な = /n/ + /a/
    sequence.push_back({kVowelA, ms2s(120)});
    sequence.push_back({kSilence, kLongPause});

    // --- 既存テスト: は さ な ま ら や わ + あいうえお ---
    sequence.insert(sequence.end(), {
        // は = /h/(80ms) + /a/(120ms)
        {kFricH_A, 3528},
        {kVowelA,  5292},
        {kSilence, kPause},
        // さ = /s/(120ms) + /a/(120ms)
        {kFricS,   5292},
        {kVowelA,  5292},
        {kSilence, kPause},
        // な = /n/(80ms) + /a/(120ms)
        {kNasalN,  3528},
        {kVowelA,  5292},
        {kSilence, kPause},
        // ま = /m/(80ms) + /a/(120ms)
        {kNasalM,  3528},
        {kVowelA,  5292},
        {kSilence, kPause},
        // ら = /ɾ/(30ms) + /a/(120ms)
        {kTapR,    1323},
        {kVowelA,  5292},
        {kSilence, kPause},
        // や = /j/(60ms) + /a/(120ms)
        {kSemiJ,   2646},
        {kVowelA,  5292},
        {kSilence, kPause},
        // わ = /w/(60ms) + /a/(120ms)
        {kSemiW,   2646},
        {kVowelA,  5292},
        {kSilence, kPause},
        // 5母音: あいうえお
        {kVowelA,  5292},
        {kVowelI,  5292},
        {kVowelU,  5292},
        {kVowelE,  5292},
        {kVowelO,  5292},
    });

    Synthesizer synth;
    std::vector<int16_t> buf;
    synth.synthesize(sequence, buf);

    if (!writeWav("output.wav", buf, static_cast<int>(kSampleRate))) {
        return 1;
    }

    std::cout << "Wrote output.wav (" << buf.size() << " samples, "
              << sequence.size() << " segments: ka ta pa + ga da ba"
              << " + kappa + kanna + ha sa na ma ra ya wa + aiueo)\n";
    return 0;
}
