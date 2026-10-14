#ifndef _COLORWHEEL_H_
#define _COLORWHEEL_H_

#include "effects/effect.h"


class ColorWheelEffect : public Effect {
private:
    uint8_t currentColor[3];
    uint16_t duration = 2000;
public:
    ColorWheelEffect(Leds& leds);
    void update(uint32_t deltaTime, Leds& leds) override;
};
#endif