#include "leds.h"
#include "esp_log.h"
#include <algorithm>
#include "effects/fadeToColor.h"

FadeToColorEffect::FadeToColorEffect(Leds& leds, uint8_t r, uint8_t g, uint8_t b) {
    fadeTo[0] = r;
    fadeTo[1] = g; 
    fadeTo[2] = b;
    
    currentColor[0] = leds.getLedColor(0)[0];
    currentColor[1] = leds.getLedColor(0)[1];
    currentColor[2] = leds.getLedColor(0)[2];
}

void FadeToColorEffect::update(uint32_t deltaTime, Leds& leds) {
    if (finished) {
        return;
    }

    timer += deltaTime;
    if (timer >= duration) {
        for (int i = 0; i < 4; i++) {
            leds.setColor(i, fadeTo[0], fadeTo[1], fadeTo[2]);
        }
        finished = true;
        return;
    }

    float t = (float)timer / duration;

    for (int i = 0; i < 4; i++) {
        uint8_t r = static_cast<uint8_t>(currentColor[0] + (fadeTo[0] - currentColor[0]) * t);
        uint8_t g = static_cast<uint8_t>(currentColor[1] + (fadeTo[1] - currentColor[1]) * t);
        uint8_t b = static_cast<uint8_t>(currentColor[2] + (fadeTo[2] - currentColor[2]) * t);

        leds.setColor(i, r, g, b);
    }
}