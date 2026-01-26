#include <stdio.h>
#include <cmath>
#include "esp_log.h"
#include <algorithm>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#include "pin.h"
#include "leds.h"
#include "serial.h"
#include "storage.h"

static const uint32_t LOOP_DELAY_MS = 20;
static const uint16_t SHORT_PRESS = 350;

extern "C" void app_main(void)
{
    const char* firmwareVersion = "0.2.0";
    ESP_LOGI("Frostlight", "Firmware Version: %s", firmwareVersion);

    Pin pins;
    Leds leds;
    Storage storage;
    LedConfig config;
    Serial serial;

    float hue = 0.0f;

    uint32_t lastTime = 0;
    uint32_t deltaTime = 0;
    uint16_t shutdownFlashCooldown = 0;

    bool shutdown = false;
    bool charging = false;
    bool interaction = false;

    uint8_t mode = 0;
    uint8_t savedMode = 0;
    uint8_t brightnessMode = 0;

    leds.clear();

    uint8_t savedColor[3] = {leds.getLedColor(0)[0],leds.getLedColor(0)[1],leds.getLedColor(0)[2]};

    if (!storage.loadLedConfig(config)) {
        leds.setBrightness(255);
        leds.setColor(0,0,0,0);
        leds.setColor(1,0,0,0);
        leds.setColor(2,0,0,0);
        leds.setColor(3,0,0,0);
        leds.startFadeToColor(255,255,255);
        savedColor[0] = 255;
        savedColor[1] = 255;
        savedColor[2] = 255;
    } else {
        leds.setBrightness(config.brightness);
        leds.setColor(0,0,0,0);
        leds.setColor(1,0,0,0);
        leds.setColor(2,0,0,0);
        leds.setColor(3,0,0,0);
        leds.startFadeToColor(config.r,config.g,config.b);
        float h, s, v;
        leds.rgbToHsv(config.r, config.g, config.b,h,s,v);
        hue = h/360.0f;
        savedColor[0] = config.r;
        savedColor[1] = config.g;
        savedColor[2] = config.b;
    }

    leds.update(0);

    while (true) {
        uint32_t now = esp_timer_get_time() / 1000;
        if (lastTime == 0) {lastTime = now;}
        deltaTime = now-lastTime;
        lastTime = now;
        pins.update(now);
        serial.update();

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
                    config.r = 255;
                    config.g = 255;
                    config.b = 255;
                    config.brightness = leds.getBrightness();
                    storage.saveLedConfig(config);
                }
            }
            if (pins.getButton2Pressed() && pins.getButton2HeldTimer() >= SHORT_PRESS) { // change color
                if (leds.getLedColor(0)[0] == 255 && leds.getLedColor(0)[1] == 255 && leds.getLedColor(0)[2] == 255 && leds.getEffectCount() == 0) { // reset from white to red
                    leds.startFadeToColor(255,0,0);
                    hue = 0.0f;
                }
                
                if (leds.getEffectCount() == 0) {
                    hue += 0.0025f; // cycle color
                    if (hue >= 1.0f) hue -= 1.0f;
                    
                    uint8_t r, g, b;
                    leds.hsvToRgb(hue, 1.0f, 1.0f, r, g, b);
                    config.r = r;
                    config.g = g;
                    config.b = b;
                    config.brightness = leds.getBrightness();
                    storage.saveLedConfig(config);

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

                config.r = leds.getLedColor(0)[0];
                config.g = leds.getLedColor(0)[1];
                config.b = leds.getLedColor(0)[2];
                config.brightness = leds.getBrightness();
                storage.saveLedConfig(config);
            }
        }

        if (mode == 2) { // charge
            if (leds.getEffectCount() == 0 && shutdown == false) { leds.startBreathEffect(); }
            if (!pins.isCharging() && charging && pins.getBatteryPercentage() >= 85) {
                mode = 3;
                leds.startFadeToColor(0,0,255);
            }
        }

        if (mode == 3) { // fully charged
            if (leds.getEffectCount() == 0 && shutdown == false) {
                leds.startBreathEffect();
            }
        }

        if (pins.isCharging() && !charging) { // check if charging
            savedMode = mode;
            const uint8_t* color = leds.getLedColor(0);
           
            if (color[0] != 0 || color[1] != 0 || color[2] != 0) {
                savedColor[0] = color[0];
                savedColor[1] = color[1]; 
                savedColor[2] = color[2];
            }
            leds.startFadeToColor(0,255,0);
            mode = 2;
        }

        if(serial.read("version")) {
            ESP_LOGI("Frostlight", "Firmware Version: %s", firmwareVersion);
        }

        if(serial.read("color")) {
            ESP_LOGI("Frostlight", "Color (RGB): %d, %d, %d", leds.getLedColor(0)[0], leds.getLedColor(0)[1], leds.getLedColor(0)[2]);
        }

        if(serial.read("brightness")) {
            ESP_LOGI("Frostlight", "Brightness: %d", leds.getBrightness());
        }

        if(serial.read("battery")) {
            ESP_LOGI("Frostlight", "Battery voltage: %.3f | Battery charge percentage: %d | Charging: %s", pins.getADC(), pins.getBatteryPercentage(), pins.isCharging() ? "true" : "false");
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
