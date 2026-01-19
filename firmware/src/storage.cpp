#include "storage.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_err.h"
#include <stdio.h>

static const char *TAG = "Storage";
static const char *LED_CONFIG_PATH = "/spiffs/led_config.txt";

Storage::Storage() : mounted(false)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "storage",
        .max_files = 3,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPIFFS mount failed (%s)", esp_err_to_name(ret));
        mounted = false;
        return;
    }
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "SPIFFS mounted: %d / %d bytes used", used, total);
    }
    mounted = true;
}

Storage::~Storage()
{
    if (mounted) {
        esp_vfs_spiffs_unregister(NULL);
        mounted = false;
    }
}

bool Storage::isReady() const
{
    return mounted;
}

bool Storage::saveLedConfig(const LedConfig &cfg)
{
    if (!mounted) {
        ESP_LOGE(TAG, "SPIFFS not mounted, cannot save config");
        return false;
    }

    FILE *f = fopen(LED_CONFIG_PATH, "w");
    if (f == nullptr) {
        ESP_LOGE(TAG, "Failed to open config file for writing");
        return false;
    }

    int written = fprintf(f, "%u %u %u %u\n",
                          static_cast<unsigned>(cfg.r),
                          static_cast<unsigned>(cfg.g),
                          static_cast<unsigned>(cfg.b),
                          static_cast<unsigned>(cfg.brightness));
    fclose(f);

    if (written < 0) {
        ESP_LOGE(TAG, "Error writing config file");
        return false;
    }

    return true;
}

bool Storage::loadLedConfig(LedConfig &config)
{
    if (!mounted) {
        ESP_LOGE(TAG, "SPIFFS not mounted, cannot load config");
        return false;
    }

    FILE *f = fopen(LED_CONFIG_PATH, "r");
    if (f == nullptr) {
        ESP_LOGW(TAG, "Config file not found, using defaults");
        config.r = 255;
        config.g = 255;
        config.b = 255;
        config.brightness = 255;
        return false;
    }

    unsigned r, g, b, brightness;
    int n = fscanf(f, "%u %u %u %u", &r, &g, &b, &brightness);
    fclose(f);

    if (n != 4 || r > 255 || g > 255 || b > 255 || brightness > 255) {
        ESP_LOGW(TAG, "Config file invalid, using defaults");
        config.r = 255;
        config.g = 255;
        config.b = 255;
        config.brightness = 128;
        return false;
    }

    config.r = static_cast<uint8_t>(r);
    config.g = static_cast<uint8_t>(g);
    config.b = static_cast<uint8_t>(b);
    config.brightness = static_cast<uint8_t>(brightness);

    ESP_LOGI(TAG, "Config loaded: R=%u G=%u B=%u Bright=%u", config.r, config.g, config.b, config.brightness);
    return true;
}
