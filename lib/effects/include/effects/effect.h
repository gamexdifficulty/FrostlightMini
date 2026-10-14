#ifndef _EFFECT_H_
#define _EFFECT_H_

class Leds;

class Effect {
protected:
    bool finished = false;
    uint32_t timer = 0;
public:
    virtual ~Effect() = default;
    virtual void update(uint32_t deltaTime, Leds& leds) = 0;
    bool isFinished() const { return finished;};
};

#endif