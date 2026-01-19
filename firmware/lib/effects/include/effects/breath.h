#ifndef _BREATH_H_
#define _BREATH_H_

#include "effects/effect.h"

class BreathEffect : public Effect {
private:
    uint8_t brightnessToBrighten = 0;
    uint16_t duration = 3000;
public:
    BreathEffect(Leds& leds);
    void update(uint32_t deltaTime, Leds& leds) override;
};
#endif