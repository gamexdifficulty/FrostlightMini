#include "storage.h"

#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_err.h"
#include <stdio.h>

static const char *TAG = "Storage";
static const char *LOG_FILE = "/spiffs/battery.csv";

Storage::Storage() : mounted(false)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
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

void Storage::logBattery(uint32_t time_ms, bool charging, float voltage)
{
    if (!mounted) {
        ESP_LOGW(TAG, "SPIFFS not mounted, skipping log");
        return;
    }

    FILE *f = fopen(LOG_FILE, "a");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open %s", LOG_FILE);
        return;
    }

    fprintf(f, "%lu,%d,%.3f\n",
            (unsigned long)time_ms,
            charging ? 1 : 0,
            voltage);

    fclose(f);
}
