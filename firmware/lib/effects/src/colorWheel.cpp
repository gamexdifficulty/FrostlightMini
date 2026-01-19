#include <cmath>
#include "leds.h"
#include "esp_log.h"
#include <algorithm>
#include "effects/colorWheel.h"

ColorWheelEffect::ColorWheelEffect(Leds& leds) {
    currentColor[0] = leds.getLedColor(0)[0];
    currentColor[1] = leds.getLedColor(0)[1];
    currentColor[2] = leds.getLedColor(0)[2];
}

void ColorWheelEffect::update(uint32_t deltaTime, Leds& leds) {
    if (finished) {
        return;
    }

    timer += deltaTime;
    if (timer >= duration) {
        for (int i = 0; i < 4; i++) {
            leds.setColor(i, currentColor[0], currentColor[1], currentColor[2]);
        }
        finished = true;
        return;
    }

    float t = (float)timer / duration;

    float hue = t * 360.0f;
    
    for (int i = 0; i < 4; i++) {
        float ledHue = fmodf(hue + (i * 90.0f), 360.0f);
        
        uint8_t r, g, b;
        leds.hsvToRgb(ledHue / 360.0f, 1.0f, 1.0f, r, g, b);
        
        leds.setColor(i, r, g, b);
    }
}