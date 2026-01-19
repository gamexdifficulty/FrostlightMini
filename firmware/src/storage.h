#pragma once
#include <stdint.h>

struct LedConfig {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t brightness;
};

class Storage {
public:
    Storage();
    ~Storage();

    bool isReady() const;

    bool saveLedConfig(const LedConfig &config);
    bool loadLedConfig(LedConfig &config);

private:
    bool mounted;
};
