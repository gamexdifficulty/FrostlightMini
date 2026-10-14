#include "leds.h"
#include "esp_log.h"
#include <algorithm>
#include "effects/breath.h"

BreathEffect::BreathEffect(Leds& leds) {
    brightnessToBrighten = leds.getBrightness();
}

void BreathEffect::update(uint32_t deltaTime, Leds& leds) {
    uint8_t newBrightness = 0;
    if (finished) {
        return;
    }

    timer += deltaTime;
    if (timer >= duration) {
        leds.setBrightness(brightnessToBrighten);
        finished = true;
        return;
    }

    float t = (float)timer / duration;

    if (t < 0.5f) {
        newBrightness = brightnessToBrighten * (1 - (t / 0.5f));
    } else {
        newBrightness = brightnessToBrighten * ((t - 0.5f) / 0.5f);
    }

    leds.setBrightness(newBrightness);
}