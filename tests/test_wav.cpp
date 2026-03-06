#define TEST_BUILD
#include "../formant.cpp"
#include "../test_framework.h"
#include <filesystem>
#include <fstream>

// Helper to read little-endian values from binary file
static uint16_t readU16(std::ifstream& ifs) {
    uint16_t v = 0;
    ifs.read(reinterpret_cast<char*>(&v), 2);
    return v;
}

static uint32_t readU32(std::ifstream& ifs) {
    uint32_t v = 0;
    ifs.read(reinterpret_cast<char*>(&v), 4);
    return v;
}

static std::string read4(std::ifstream& ifs) {
    char buf[4];
    ifs.read(buf, 4);
    return std::string(buf, 4);
}

static const char* kTestWav = "/tmp/test_wav_formant.wav";

// 1. writeWav returns true for normal case
REGISTER_TEST(writeWav_returns_true) {
    std::vector<int16_t> samples(100, 1000);
    bool ok = writeWav(kTestWav, samples, 44100);
    ASSERT_TRUE(ok);
    std::filesystem::remove(kTestWav);
}

// 2. Output file exists after writeWav
REGISTER_TEST(writeWav_file_exists) {
    std::vector<int16_t> samples(100, 1000);
    writeWav(kTestWav, samples, 44100);
    ASSERT_TRUE(std::filesystem::exists(kTestWav));
    std::filesystem::remove(kTestWav);
}

// 3. RIFF header (first 4 bytes)
REGISTER_TEST(writeWav_riff_header) {
    std::vector<int16_t> samples(100, 1000);
    writeWav(kTestWav, samples, 44100);

    std::ifstream ifs(kTestWav, std::ios::binary);
    ASSERT_TRUE(ifs.good());
    ASSERT_EQ(read4(ifs), std::string("RIFF"));

    ifs.close();
    std::filesystem::remove(kTestWav);
}

// 4. File size field (bytes 4-7) == 36 + samples.size()*2
REGISTER_TEST(writeWav_file_size_field) {
    std::vector<int16_t> samples(100, 1000);
    writeWav(kTestWav, samples, 44100);

    std::ifstream ifs(kTestWav, std::ios::binary);
    ifs.seekg(4);
    uint32_t fileSize = readU32(ifs);
    ASSERT_EQ(fileSize, static_cast<uint32_t>(36 + samples.size() * 2));

    ifs.close();
    std::filesystem::remove(kTestWav);
}

// 5. Bytes 8-11 == "WAVE"
REGISTER_TEST(writeWav_wave_tag) {
    std::vector<int16_t> samples(100, 1000);
    writeWav(kTestWav, samples, 44100);

    std::ifstream ifs(kTestWav, std::ios::binary);
    ifs.seekg(8);
    ASSERT_EQ(read4(ifs), std::string("WAVE"));

    ifs.close();
    std::filesystem::remove(kTestWav);
}

// 6. fmt chunk: PCM=1, channels=1, sampleRate=44100, bits=16
REGISTER_TEST(writeWav_fmt_chunk) {
    std::vector<int16_t> samples(100, 1000);
    writeWav(kTestWav, samples, 44100);

    std::ifstream ifs(kTestWav, std::ios::binary);
    // fmt chunk starts at byte 12
    ifs.seekg(12);
    ASSERT_EQ(read4(ifs), std::string("fmt "));   // chunk ID
    ASSERT_EQ(readU32(ifs), 16u);                  // chunk size
    ASSERT_EQ(readU16(ifs), 1u);                   // PCM format
    ASSERT_EQ(readU16(ifs), 1u);                   // mono
    ASSERT_EQ(readU32(ifs), 44100u);               // sample rate
    ASSERT_EQ(readU32(ifs), 88200u);               // byte rate (44100 * 2)
    ASSERT_EQ(readU16(ifs), 2u);                   // block align
    ASSERT_EQ(readU16(ifs), 16u);                  // bits per sample

    ifs.close();
    std::filesystem::remove(kTestWav);
}

// 7. data chunk size == samples.size() * 2
REGISTER_TEST(writeWav_data_chunk_size) {
    std::vector<int16_t> samples(100, 1000);
    writeWav(kTestWav, samples, 44100);

    std::ifstream ifs(kTestWav, std::ios::binary);
    // data chunk starts at byte 36
    ifs.seekg(36);
    ASSERT_EQ(read4(ifs), std::string("data"));
    uint32_t dataSize = readU32(ifs);
    ASSERT_EQ(dataSize, static_cast<uint32_t>(samples.size() * 2));

    ifs.close();
    std::filesystem::remove(kTestWav);
}

// 8. Empty samples produce a valid WAV
REGISTER_TEST(writeWav_empty_samples) {
    std::vector<int16_t> samples;
    bool ok = writeWav(kTestWav, samples, 44100);
    ASSERT_TRUE(ok);

    std::ifstream ifs(kTestWav, std::ios::binary);
    ASSERT_EQ(read4(ifs), std::string("RIFF"));
    uint32_t fileSize = readU32(ifs);
    ASSERT_EQ(fileSize, 36u);  // 36 + 0
    ASSERT_EQ(read4(ifs), std::string("WAVE"));

    // data chunk size should be 0
    ifs.seekg(36);
    ASSERT_EQ(read4(ifs), std::string("data"));
    ASSERT_EQ(readU32(ifs), 0u);

    ifs.close();
    std::filesystem::remove(kTestWav);
}

// 9. Writing to non-existent directory returns false
REGISTER_TEST(writeWav_nonexistent_dir_returns_false) {
    std::vector<int16_t> samples(10, 500);
    bool ok = writeWav("/tmp/nonexistent_dir_xyz/test.wav", samples, 44100);
    ASSERT_FALSE(ok);
}

int main() {
    return run_all_tests("WAVWriter");
}
