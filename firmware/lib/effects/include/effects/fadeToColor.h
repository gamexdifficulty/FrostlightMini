#ifndef _FADETOCOLOR_H_
#define _FADETOCOLOR_H_

#include "effects/effect.h"


class FadeToColorEffect : public Effect {
private:
    uint8_t fadeTo[3];
    uint8_t currentColor[3];
    uint16_t duration = 1000;
public:
    FadeToColorEffect(Leds& leds, uint8_t r, uint8_t g, uint8_t b);
    void update(uint32_t deltaTime, Leds& leds) override;
};
#endif