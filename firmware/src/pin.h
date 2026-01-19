#ifndef _PIN_H_
#define _PIN_H_

#include <cstdint>

class Pin {
private:
    bool button1Pressed = false;
    bool button2Pressed = false;
    bool button1Released = false;
    bool button2Released = false;
    bool button1Initialized = false;
    bool button2Initialized = false;
    uint32_t button1HeldTimer = 0;
    uint32_t button2HeldTimer = 0;
    uint32_t button1HeldStartTimer = 0;
    uint32_t button2HeldStartTimer = 0;
public:
    Pin();
    void update(uint32_t timer);
    void turnOff();

    float getADC();
    bool isCharging();
    int getBatteryPercentage();

    bool getButton1Pressed();
    bool getButton2Pressed();
    bool getButton1Released();
    bool getButton2Released();

    uint32_t getButton1HeldTimer();
    uint32_t getButton2HeldTimer();
};

#endif