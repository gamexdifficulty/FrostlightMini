#include <stdio.h>
#include <cmath>
#include "esp_log.h"
#include <algorithm>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#include "pin.h"
#include "leds.h"
#include "storage.h"

static const uint32_t LOOP_DELAY_MS = 20;
static const uint16_t SHORT_PRESS = 350;

extern "C" void app_main(void)
{
    const char* firmwareVersion = "0.1.0";
    ESP_LOGI("Frostlight", "Firmware Version: %s", firmwareVersion);

    Pin pins;
    Leds leds;

    uint32_t lastTime = 0;
    uint32_t deltaTime = 0;
    uint16_t shutdownFlashCooldown = 0;

    bool shutdown = false;
    bool charging = false;
    bool interaction = false;

    uint8_t savedColor[3] = {255, 255, 255};

    uint8_t mode = 0;
    uint8_t savedMode = 0;
    uint8_t brightnessMode = 0;

    leds.clear();
    leds.setBrightness(255);
    leds.setColor(0,255,255,255);
    leds.setColor(1,255,255,255);
    leds.setColor(2,255,255,255);
    leds.setColor(3,255,255,255);
    leds.update(0);

    while (true) {
        uint32_t now = esp_timer_get_time() / 1000;
        if (lastTime == 0) {lastTime = now;}
        deltaTime = now-lastTime;
        lastTime = now;
        pins.update(now);

        if (pins.getBatteryPercentage() <= 0) { shutdown = true; }
        if (pins.getButton1HeldTimer() >= 1000 && shutdownFlashCooldown == 0) { 
            leds.startShortFlashEffect(); 
            shutdownFlashCooldown = 3000;
        }
        if (shutdownFlashCooldown != 0) { shutdownFlashCooldown = std::max<uint16_t>(0,shutdownFlashCooldown-deltaTime); }
        if (pins.getButton1Released() || pins.getButton2Released()) { interaction = true; } else { interaction = false; }
        if (pins.getButton1Released() && pins.getButton1HeldTimer() >= 1000) { shutdown = true; }
        if (pins.getButton1Released() && pins.getButton1HeldTimer() <= SHORT_PRESS) {
            if (mode < 2 && !charging && interaction) {
                mode = mode == 0 ? 1 : 0;
                if (mode == 0) {
                    leds.startColorWheel();
                }
                if (mode == 1) {
                    brightnessMode = 0;
                    leds.startShortFlashEffect();
                }
            }
        }

        if (mode == 0) { // color
            if (pins.getButton2Released() && pins.getButton2HeldTimer() < SHORT_PRESS) { 
                if (leds.getLedColor(0)[0] != 255 || leds.getLedColor(0)[1] != 255 || leds.getLedColor(0)[2] != 255) { // set to white
                    leds.startFadeToColor(255,255,255);
                }
            }
            if (pins.getButton2Pressed() && pins.getButton2HeldTimer() >= SHORT_PRESS) { // change color
                static float hue = 0.0f;
                
                if (leds.getLedColor(0)[0] == 255 && leds.getLedColor(0)[1] == 255 && leds.getLedColor(0)[2] == 255 && leds.getEffectCount() == 0) { // reset from white to red
                    leds.startFadeToColor(255,0,0);
                    hue = 0.0f;
                }
                
                if (leds.getEffectCount() == 0) {
                    hue += 0.0025f; // cycle color
                    if (hue >= 1.0f) hue -= 1.0f;
                    
                    uint8_t r, g, b;
                    leds.hsvToRgb(hue, 1.0f, 1.0f, r, g, b);
                    for (int i = 0; i < 4; i++) {
                        leds.setColor(i, r, g, b);
                    }
                }
            }

        }
        if (mode == 1) { // brightness up/down
            if (pins.getButton2Released() && pins.getButton2HeldTimer() < SHORT_PRESS) { 
                brightnessMode = brightnessMode == 0 ? 1 : 0;
                if (brightnessMode == 0) { leds.startShortFlashEffect(); }
                if (brightnessMode == 1) { leds.startShortFlashEffect(); }
            }
            if (pins.getButton2Pressed() && pins.getButton2HeldTimer() >= SHORT_PRESS) {
                uint8_t brightness = leds.getBrightness();
                if (brightnessMode == 0) { leds.setBrightness(std::max<uint8_t>(2,brightness - 1)); }     // brightness down
                if (brightnessMode == 1) { leds.setBrightness(std::min<uint8_t>(254,brightness + 1)); }   // brightness up
            }
        }

        if (mode == 2) { // charge
            if (leds.getEffectCount() == 0) { leds.startBreathEffect(); }
            ESP_LOGI("Frostlight", "%d", pins.getBatteryPercentage());
            if (!pins.isCharging() && charging && pins.getBatteryPercentage() >= 85) {
                mode = 3;
                leds.startFadeToColor(0,0,255);
            }
        }

        if (mode == 3) { // fully charged
            if (leds.getEffectCount() == 0) {
                leds.startBreathEffect();
            }
        }

        if (pins.isCharging() && !charging) { // check if charging
            savedMode = mode;
            const uint8_t* color = leds.getLedColor(0);
            savedColor[0] = color[0];
            savedColor[1] = color[1]; 
            savedColor[2] = color[2];

            leds.startFadeToColor(0,255,0);
            mode = 2;
            ESP_LOGI("Frostlight", "Firmware Version: %s", firmwareVersion);
        }

        if (((!pins.isCharging() && charging) && mode == 2) || (interaction && mode > 1)) { // unplugged or interacted with
            mode = savedMode;
            leds.startFadeToColor(savedColor[0], savedColor[1], savedColor[2]);
        }

        charging = pins.isCharging();

        if (shutdown) { // shutoff
            leds.darkenOff();
            if (leds.getBrightness() <= 0) {
                pins.turnOff();
            }
        }

        leds.update(deltaTime);
        vTaskDelay(pdMS_TO_TICKS(LOOP_DELAY_MS));
    }
}
