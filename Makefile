CXX       ?= c++
CXXFLAGS   = -std=c++20 -O2 -Wall -Wextra
TARGET     = formant

TESTS = test_resonator test_noisegen test_impulsetrain test_lerp \
        test_helpers test_phoneme_data test_synth_basic test_synth_source \
        test_wav test_integration \
        test_mixed_source test_new_fricatives test_affricates \
        test_ha_row test_sa_row test_za_row \
        test_text_to_phoneme test_youon_special test_integration_ms6
TEST_BINS = $(addprefix tests/, $(TESTS))

$(TARGET): formant.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

tests/%: tests/%.cpp formant.cpp test_framework.h
	$(CXX) $(CXXFLAGS) -Wno-unused-function -o $@ $<

test: $(TEST_BINS)
	@failed=0; for t in $(TEST_BINS); do ./$$t || failed=$$((failed+1)); done; \
	echo ""; \
	if [ $$failed -eq 0 ]; then echo "ALL TEST SUITES PASSED"; \
	else echo "$$failed TEST SUITE(S) FAILED"; exit 1; fi

clean:
	rm -f $(TARGET) $(TEST_BINS) *.wav

.PHONY: clean test
