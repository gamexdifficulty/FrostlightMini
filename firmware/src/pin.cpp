#include "pin.h"
#include "math.h"

#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"

/* GPIO definitions */
#define PWR_GPIO GPIO_NUM_4
#define CHRG_GPIO GPIO_NUM_6
#define BUTTON1GPIO GPIO_NUM_7
#define BUTTON2GPIO GPIO_NUM_9

/* ADC definitions */
#define ADC_CHANNEL ADC1_CHANNEL_3
#define ADC_WIDTH ADC_WIDTH_BIT_12
#define ADC_ATTEN ADC_ATTEN_DB_11
#define DEFAULT_VREF 1100

static esp_adc_cal_characteristics_t adc_chars;

Pin::Pin() {
    /* Power GPIO */
    gpio_config_t pwr_conf = {};
    pwr_conf.pin_bit_mask = (1ULL << PWR_GPIO);
    pwr_conf.mode = GPIO_MODE_OUTPUT;
    pwr_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    pwr_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    pwr_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&pwr_conf);
    gpio_set_level(PWR_GPIO, 1);

    /* CHRG pin */
    gpio_config_t chrg_conf = {};
    chrg_conf.pin_bit_mask = (1ULL << CHRG_GPIO);
    chrg_conf.mode = GPIO_MODE_INPUT;
    chrg_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    chrg_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    chrg_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&chrg_conf);

    /* Button GPIOs */
    gpio_config_t btn_conf = {};
    btn_conf.pin_bit_mask =
        (1ULL << BUTTON1GPIO) |
        (1ULL << BUTTON2GPIO);
    btn_conf.mode = GPIO_MODE_INPUT;
    btn_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    btn_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    btn_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&btn_conf);

    /* ADC setup */
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);

    esp_adc_cal_characterize(
        ADC_UNIT_1,
        ADC_ATTEN,
        ADC_WIDTH,
        DEFAULT_VREF,
        &adc_chars);
}

void Pin::update(uint32_t timer) {
    bool btn1Now = (gpio_get_level(BUTTON1GPIO) == 0);
    bool btn2Now = (gpio_get_level(BUTTON2GPIO) == 0);

    if (!button1Initialized) {
        button1Initialized = !btn1Now;
    } else {
        button1Released = (button1Pressed && !btn1Now);
        button1Pressed = btn1Now;
    }

    if (!button2Initialized) {
        button2Initialized = !btn2Now;
    } else {
        button2Released = (button2Pressed && !btn2Now);
        button2Pressed = btn2Now;
    }

    if (button1Pressed && button1HeldStartTimer == 0) { button1HeldStartTimer = timer; }
    if (button2Pressed && button2HeldStartTimer == 0) { button2HeldStartTimer = timer; }

    if (button1HeldStartTimer != 0) { button1HeldTimer = timer - button1HeldStartTimer; }
    if (button2HeldStartTimer != 0) { button2HeldTimer = timer - button2HeldStartTimer; }

    if (!button1Pressed and !button1Released and button1HeldStartTimer > 0) {
        button1HeldStartTimer = 0;
        button1HeldTimer = 0;
    }

    if (!button2Pressed and !button2Released and button2HeldStartTimer > 0) {
        button2HeldStartTimer = 0;
        button2HeldTimer = 0;
    }

    if (monitorBattery && timer - batteryMonitoringTimer > 3000) {
        ESP_LOGI("Frostlight", "Battery voltage: %.3f | Battery charge percentage: %d | Charging: %s", getADC(), getBatteryPercentage(),isCharging() ? "true" : "false");
        batteryMonitoringTimer = timer;
    }
}

bool Pin::getButton1Pressed() { return button1Pressed; }
bool Pin::getButton2Pressed() { return button2Pressed; }
bool Pin::getButton1Released() { return button1Released; }
bool Pin::getButton2Released() { return button2Released; }

uint32_t Pin::getButton1HeldTimer() { return button1HeldTimer; }
uint32_t Pin::getButton2HeldTimer() { return button2HeldTimer; }

float Pin::getADC() {
    int raw = adc1_get_raw(ADC_CHANNEL);
    uint32_t mv = esp_adc_cal_raw_to_voltage(raw, &adc_chars);

    float adc_voltage = mv / 1000.0f;
    return adc_voltage * 2.0f;
}

int Pin::getBatteryPercentage() {
    float v = getADC();
    int percent = 100.0f * powf((v - 3.3f) / 0.9f, 1.6f);
    
    if (percent > 100) percent = 100;
    if (percent < 0) percent = 0;
    
    return percent;
}

bool Pin::isCharging() { return gpio_get_level(CHRG_GPIO) == 0; }
void Pin::turnOff() { gpio_set_level(PWR_GPIO, 0); }