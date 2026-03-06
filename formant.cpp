// formant.cpp — MS1: Pipeline skeleton, vowel "a" (あ)
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
constexpr double kDuration    = 1.0;
constexpr int    kNumSamples  = static_cast<int>(kSampleRate * kDuration);
constexpr double kPi          = std::numbers::pi;

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
    // Formant parameters for vowel "a" (あ)
    //          freq   bw
    struct FParam { double freq; double bw; };
    constexpr std::array<FParam, 3> formants = {{
        {800.0,  80.0},
        {1200.0, 100.0},
        {2600.0, 120.0},
    }};

    // Set up resonators
    std::array<Resonator, 3> res{};
    for (std::size_t i = 0; i < formants.size(); ++i) {
        res[i].set(formants[i].freq, formants[i].bw, kSampleRate);
    }

    // Synthesise
    ImpulseTrain src;
    std::vector<int16_t> buf(kNumSamples);

    for (int n = 0; n < kNumSamples; ++n) {
        double s = src.next(kF0, kSampleRate);

        // Cascade: F1 -> F2 -> F3
        s = res[0].process(s);
        s = res[1].process(s);
        s = res[2].process(s);

        // Scale and clamp to int16 range
        double out = s * kAmplitude;
        out = std::clamp(out, -32768.0, 32767.0);
        buf[n] = static_cast<int16_t>(out);
    }

    // Write WAV
    if (!writeWav("output.wav", buf, static_cast<int>(kSampleRate))) {
        return 1;
    }

    std::cout << "Wrote output.wav\n";
    return 0;
}
