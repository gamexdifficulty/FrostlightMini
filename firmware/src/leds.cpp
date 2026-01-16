#include "leds.h"
#include "esp_log.h"
#include "esp_err.h"

Leds::Leds() {
    led_strip_config_t strip_config = {
        .strip_gpio_num = ledGPIOPin,
        .max_leds = ledCount,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags = {.invert_out = false }
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 64,
        .flags = {.with_dma = false }
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &strip));
}

void Leds::update() {
    led_strip_refresh(strip);
}

void Leds::clear() {
    led_strip_clear(strip);
}

int Leds::getCount() {
    return this->ledCount;
}

void Leds::setBrightness(char brightness) {
    this->brightness = brightness;
    for (int i = 0; i < 4; i++) {
        setColor(i,ledColor[i][0],ledColor[i][1],ledColor[i][2]);
    }
}

void Leds::darkenOff() {
    if (this->effect != 1){
        this->effect = 1;
    }
}

char Leds::getBrightness() {
    return this->brightness;
}

void Leds::setColor(int led, char red, char green, char blue) {
    ledColor[led][0] = red;
    ledColor[led][1] = green;
    ledColor[led][2] = blue;
    led_strip_set_pixel(
        strip, 
        led, 
        red*(this->brightness/255),
        green*(this->brightness/255),
        blue*(this->brightness/255)
    );
}
