#ifndef _LEDS_H_
#define _LEDS_H_

#include "led_strip.h"

class Effect;

class Leds {
private:
    uint8_t  ledCount = 4;
    uint8_t  ledGPIOPin = 10;
    uint8_t  brightness = 255;
    uint8_t  ledColor[4][3];
    uint8_t  maxEffectCount = 5;
    Effect*  effectQueue[5];
    uint8_t  effectCount = 0;
    led_strip_handle_t strip;

public:
    Leds();
    void update(uint32_t deltaTime);
    void clear();
    void darkenOff();
    void setBrightness(char brightness);
    void setColor(uint8_t led, char red, char green, char blue);
    void hsvToRgb(float h, float s, float v, uint8_t& r, uint8_t& g, uint8_t& b);
    void rgbToHsv(uint8_t r, uint8_t g, uint8_t b, float& h, float& s, float& v);

    // effects
    void startShortFlashEffect();
    void startFadeToColor(uint8_t r, uint8_t g, uint8_t b);
    void startColorWheel();
    void startBreathEffect();

    uint8_t getCount()                  const   { return this->ledCount; };
    uint8_t getEffectCount()            const   { return this->effectCount; };
    uint8_t getBrightness()             const   { return this->brightness; };
    uint8_t* getLedColor(uint8_t led) { return (led < ledCount) ? this->ledColor[led] : nullptr; }
};

#endif