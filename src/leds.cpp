#include "leds.h"
#include <cmath>
#include "esp_log.h"
#include "esp_err.h"

#include "leds.h"
#include "effects/effect.h"
#include "effects/shortFlash.h"
#include "effects/fadeToColor.h"
#include "effects/colorWheel.h"
#include "effects/breath.h"

Leds::Leds() {
    for (uint8_t i = 0; i < maxEffectCount; ++i) {
        effectQueue[i] = nullptr;
    }

    led_strip_config_t strip_config = {
        .strip_gpio_num = ledGPIOPin,
        .max_leds = ledCount,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags = {.invert_out = false }
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 64,
        .flags = {.with_dma = false }
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &strip));
}

void Leds::update(uint32_t deltaTime) {
    if (effectCount != 0) {
        effectQueue[0]->update(deltaTime,*this);

        if (effectQueue[0]->isFinished()) {
            delete effectQueue[0];
            
            for (uint8_t i = 1; i < effectCount; ++i) {
                effectQueue[i - 1] = effectQueue[i];
            }

            effectQueue[effectCount - 1] = nullptr;
            effectCount--;
        }
    }

    led_strip_refresh(strip);
}

void Leds::clear() { led_strip_clear(strip);}

void Leds::setBrightness(char brightness) {
    this->brightness = brightness;
    for (int i = 0; i < 4; i++) {
        setColor(i,ledColor[i][0],ledColor[i][1],ledColor[i][2]);
    }
}

void Leds::darkenOff() {
    setBrightness(getBrightness()-3 < 0 ? 0 : getBrightness()-3);
}

void Leds::startShortFlashEffect() {
    if (effectCount >= maxEffectCount) { return; }
    effectQueue[effectCount++] = new ShortFlashEffect(*this);
}

void Leds::startFadeToColor(uint8_t r, uint8_t g, uint8_t b) {
    if (effectCount >= maxEffectCount) { return; }
    effectQueue[effectCount++] = new FadeToColorEffect(*this, r, g, b);
}

void Leds::startColorWheel() {
    if (effectCount >= maxEffectCount) { return; }
    effectQueue[effectCount++] = new ColorWheelEffect(*this);
}

void Leds::startBreathEffect() {
    if (effectCount >= maxEffectCount) { return; }
    effectQueue[effectCount++] = new BreathEffect(*this);
}

void Leds::setColor(uint8_t led, char red, char green, char blue) {
    ledColor[led][0] = red;
    ledColor[led][1] = green;
    ledColor[led][2] = blue;
    float scale = this->brightness / 255.0f;

    led_strip_set_pixel(
        strip,
        led,
        (uint8_t)(red   * scale),
        (uint8_t)(green * scale),
        (uint8_t)(blue  * scale)
    );
}

void Leds::hsvToRgb(float h, float s, float v, uint8_t& r, uint8_t& g, uint8_t& b) {
    float c = v * s;
    float x = c * (1.0f - fabsf(fmodf(h * 6.0f, 2.0f) - 1.0f));
    float m = v - c;
    
    float r1, g1, b1;
    
    if (h < 1.0f/6.0f) { r1 = c; g1 = x; b1 = 0; }
    else if (h < 2.0f/6.0f) { r1 = x; g1 = c; b1 = 0; }
    else if (h < 3.0f/6.0f) { r1 = 0; g1 = c; b1 = x; }
    else if (h < 4.0f/6.0f) { r1 = 0; g1 = x; b1 = c; }
    else if (h < 5.0f/6.0f) { r1 = x; g1 = 0; b1 = c; }
    else { r1 = c; g1 = 0; b1 = x; }
    
    r = static_cast<uint8_t>((r1 + m) * 255.0f);
    g = static_cast<uint8_t>((g1 + m) * 255.0f);
    b = static_cast<uint8_t>((b1 + m) * 255.0f);
}

void Leds::rgbToHsv(uint8_t r, uint8_t g, uint8_t b, float& h, float& s, float& v) {
    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;
    
    float maxVal = rf;
    if (gf > maxVal) maxVal = gf;
    if (bf > maxVal) maxVal = bf;
    
    float minVal = rf;
    if (gf < minVal) minVal = gf;
    if (bf < minVal) minVal = bf;
    
    float delta = maxVal - minVal;
    
    if (delta == 0.0f) {
        h = 0.0f;
    } else {
        if (maxVal == rf) {
            h = fmodf((gf - bf) / delta, 6.0f);
        } else if (maxVal == gf) {
            h = (bf - rf) / delta + 2.0f;
        } else {
            h = (rf - gf) / delta + 4.0f;
        }
        h *= 60.0f;
        if (h < 0.0f) h += 360.0f;
    }
    
    s = (maxVal == 0.0f) ? 0.0f : (delta / maxVal);
    v = maxVal;
}

