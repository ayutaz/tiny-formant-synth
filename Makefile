CXX       ?= c++
CXXFLAGS   = -std=c++17 -O2 -Wall -Wextra
TARGET     = formant

$(TARGET): formant.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) *.wav

.PHONY: clean
