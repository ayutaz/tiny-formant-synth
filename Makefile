CXX       ?= c++
CXXFLAGS   = -std=c++20 -O2 -Wall -Wextra
TARGET     = formant

$(TARGET): formant.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) *.wav

.PHONY: clean
