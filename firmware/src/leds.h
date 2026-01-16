#ifndef _LEDS_H_
#define _LEDS_H_

#include "led_strip.h"

class Leds {
private:
    char ledCount = 4;
    char ledGPIOPin = 10;
    char brightness;
    char effect;
    char ledColor[4][3];
    led_strip_handle_t strip;

public:
    Leds();
    void update();
    void clear();
    int getCount();
    char getBrightness();
    void darkenOff();
    void setBrightness(char brightness);
    void setColor(int led, char red, char green, char blue);
};

#endif