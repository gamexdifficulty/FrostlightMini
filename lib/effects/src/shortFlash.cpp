#include "leds.h"
#include "esp_log.h"
#include <algorithm>
#include "effects/shortFlash.h"

ShortFlashEffect::ShortFlashEffect(Leds& leds) {
    brightnessToBrighten = leds.getBrightness();
    brightnessToDarken = std::max<uint8_t>(2,brightnessToBrighten-100);
}

void ShortFlashEffect::update(uint32_t deltaTime, Leds& leds) {
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
        newBrightness = brightnessToBrighten + (brightnessToDarken - brightnessToBrighten) * (t / 0.5f);
    } else {
        newBrightness = brightnessToDarken + (brightnessToBrighten - brightnessToDarken) * ((t - 0.5f) / 0.5f);
    }

    leds.setBrightness(newBrightness);
}