#include "driver/usb_serial_jtag.h"
#include <stdio.h>
#include <string.h>
#include "serial.h"

Serial::Serial(){
    usb_serial_jtag_driver_config_t config = {};
    config.rx_buffer_size = 256;
    config.tx_buffer_size = 256;

    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&config));
}

void Serial::update() {
    len = usb_serial_jtag_read_bytes(buf, sizeof(buf), 20 / portTICK_PERIOD_MS);
}

bool Serial::read(const char* command) {
    if (len > 0) {
        if (len >= strlen(command) && memcmp(buf, command, strlen(command)) == 0) {
            return true;
        }
    }
    return false;
}

void Serial::write(const char* output) {
    usb_serial_jtag_write_bytes((const uint8_t *)output, strlen(output), portMAX_DELAY);
}