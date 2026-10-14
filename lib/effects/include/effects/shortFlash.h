#ifndef _SHORTFLASH_H_
#define _SHORTFLASH_H_

#include "effects/effect.h"


class ShortFlashEffect : public Effect {
private:
    uint8_t brightnessToDarken = 0;
    uint8_t brightnessToBrighten = 0;
    uint16_t duration = 1000;
public:
    ShortFlashEffect(Leds& leds);
    void update(uint32_t deltaTime, Leds& leds) override;
};
#endif