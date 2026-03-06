// formant.cpp — MS2: Five vowels with formant transitions (あいうえお)
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

struct FormantParams {
    double f1, f2, f3;
    double bw1, bw2, bw3;
};

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

// ============ Phoneme Data ============

constexpr FormantParams kVowelA = {800, 1200, 2600, 80, 100, 120};
constexpr FormantParams kVowelI = {300, 2300, 3000, 80, 120, 150};
constexpr FormantParams kVowelU = {350, 1300, 2500, 80, 100, 120};
constexpr FormantParams kVowelE = {500, 1900, 2600, 80, 100, 120};
constexpr FormantParams kVowelO = {500,  800, 2400, 80, 100, 120};

struct PhonemeEntry {
    FormantParams params;
    int duration_samples;
};

// ============ Source Generation ============

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
    // Phoneme sequence: あ い う え お (each 200 ms)
    constexpr int kSegDur = 8820;              // 200 ms at 44.1 kHz
    std::vector<PhonemeEntry> sequence = {
        {kVowelA, kSegDur},
        {kVowelI, kSegDur},
        {kVowelU, kSegDur},
        {kVowelE, kSegDur},
        {kVowelO, kSegDur},
    };

    int totalSamples = 0;
    for (auto& e : sequence) totalSamples += e.duration_samples;

    // Resonators & source — kept alive across segments for continuity
    std::array<Resonator, 3> res{};
    ImpulseTrain src;
    std::vector<int16_t> buf(totalSamples);

    int pos = 0;
    for (std::size_t seg = 0; seg < sequence.size(); ++seg) {
        const auto& cur = sequence[seg].params;
        const auto& nxt = (seg + 1 < sequence.size())
                              ? sequence[seg + 1].params : cur;
        int dur = sequence[seg].duration_samples;

        for (int n = 0; n < dur; ++n) {
            // Update filter coefficients once per frame
            if (n % kFrameSize == 0) {
                double t = static_cast<double>(n) / dur;
                double blend = (t > 0.7) ? (t - 0.7) / 0.3 : 0.0;

                double f1  = cur.f1  + (nxt.f1  - cur.f1)  * blend;
                double f2  = cur.f2  + (nxt.f2  - cur.f2)  * blend;
                double f3  = cur.f3  + (nxt.f3  - cur.f3)  * blend;
                double bw1 = cur.bw1 + (nxt.bw1 - cur.bw1) * blend;
                double bw2 = cur.bw2 + (nxt.bw2 - cur.bw2) * blend;
                double bw3 = cur.bw3 + (nxt.bw3 - cur.bw3) * blend;

                res[0].set(f1, bw1, kSampleRate);
                res[1].set(f2, bw2, kSampleRate);
                res[2].set(f3, bw3, kSampleRate);
            }

            // Source -> Cascade: F1 -> F2 -> F3
            double s = src.next(kF0, kSampleRate);
            s = res[0].process(s);
            s = res[1].process(s);
            s = res[2].process(s);

            double out = s * kAmplitude;
            out = std::clamp(out, -32768.0, 32767.0);
            buf[pos++] = static_cast<int16_t>(out);
        }
    }

    // Write WAV
    if (!writeWav("output.wav", buf, static_cast<int>(kSampleRate))) {
        return 1;
    }

    std::cout << "Wrote output.wav (" << totalSamples << " samples, "
              << sequence.size() << " vowels)\n";
    return 0;
}
