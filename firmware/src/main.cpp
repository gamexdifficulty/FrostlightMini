#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#include "pin.h"
#include "leds.h"
#include "storage.h"

static const char *TAG = "main";

static const uint32_t HEARTBEAT_MS = 60000;
static const uint32_t LOOP_DELAY_MS = 20;
static const uint32_t BUTTON_SHORT_PRESS_MS = 200;
static const uint32_t BUTTON_HOLD_MS = 200;

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Starting App");

    Pin pin;
    Leds leds;

    leds.clear();
    leds.setBrightness(255);
    leds.update();

    while (true) {
        uint32_t now = esp_timer_get_time() / 1000;
        pin.update(now);

        if (pin.isButton1Released() && pin.getButton1HeldTimer() > 3000) { pin.turnOff(); }

        vTaskDelay(pdMS_TO_TICKS(LOOP_DELAY_MS));
    }
}
